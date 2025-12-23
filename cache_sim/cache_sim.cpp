#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <random>

using namespace std;

// configuration and constants
struct Config {
    int cache_size;
    int block_size;
    int associativity; // 0 denotes fully associative
    string replacement_policy;
    string write_policy;
    int write_allocate;
};

// cache block structure
struct CacheBlock {
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    long long last_access_time = 0; // LRU
    long long insertion_time = 0;   // FIFO
};

// statistics
struct Stats {
    long long total_loads = 0;
    long long total_stores = 0;
    long long load_hits = 0;
    long long load_misses = 0;
    long long store_hits = 0;
    long long store_misses = 0;
    long long compulsory_misses = 0;
    long long capacity_misses = 0;
    long long conflict_misses = 0;
    long long memory_reads = 0;
    long long memory_writes = 0;
};

// glob variables
Config config;
Stats stats;
long long global_time = 0;
set<uint32_t> seen_blocks; // compulsory misses

// helper functions
uint32_t get_block_address(uint32_t addr) {
    return addr / config.block_size;
}

// sim class
class CacheSimulator {
    vector<vector<CacheBlock>> sets;
    int num_sets;
    int ways;
    int index_bits;
    int block_offset_bits;

public:
    CacheSimulator(Config cfg) {
        if (cfg.associativity == 0) { // fully associative
            num_sets = 1;
            ways = cfg.cache_size / cfg.block_size;
        }
        else { // direct mapped or N-Way
            num_sets = cfg.cache_size / (cfg.block_size * cfg.associativity);
            ways = cfg.associativity;
        }

        sets.resize(num_sets, vector<CacheBlock>(ways));

        // bitwise calculations
        block_offset_bits = log2(cfg.block_size);
        index_bits = log2(num_sets);
    }

    void access(char type, uint32_t addr) {
        global_time++;
        uint32_t block_addr = get_block_address(addr);
        uint32_t index = (block_addr) % num_sets;
        uint32_t tag = block_addr / num_sets; // simplified tag calculation

        if (config.associativity == 0) { // index is always 0 --> Fully Associative
            index = 0;
            tag = block_addr;
        }

        bool hit = false;
        int hit_way = -1;

        // hit check
        for (int i = 0; i < ways; i++) {
            if (sets[index][i].valid && sets[index][i].tag == tag) {
                hit = true;
                hit_way = i;
                break;
            }
        }

        // update hit
        if (type == 'R') {
            stats.total_loads++;
            if (hit) stats.load_hits++; else stats.load_misses++;
        }
        else {
            stats.total_stores++;
            if (hit) stats.store_hits++; else stats.store_misses++;
        }

        // hit case
        if (hit) {
            // update LRU
            if (config.replacement_policy == "LRU") {
                sets[index][hit_way].last_access_time = global_time;
            }

            // Write-Through accesses logic
            if (type == 'W') {
                if (config.write_policy == "WRITE_THROUGH") {
                    stats.memory_writes++;
                }
                else {
                    // Write-Back
                    sets[index][hit_way].dirty = true;
                }
            }
        }
        // miss case
        else {
            // miss classification
            if (seen_blocks.find(block_addr) == seen_blocks.end()) {
                stats.compulsory_misses++;
                seen_blocks.insert(block_addr);
            }
            else {
                // full cache implies capacity miss otherwise conflict
                if (config.associativity == 0) stats.capacity_misses++;
                else stats.conflict_misses++;
            }

            // memory read
            bool bring_to_cache = true;
            if (type == 'W' && config.write_allocate == 0) {
                bring_to_cache = false; // no-write-allocate
                stats.memory_writes++; // direct memory write
            }
            else {
                stats.memory_reads++; // fetch block
            }

            if (bring_to_cache) {
                // replacement way
                int replace_way = -1;

                // empty check
                for (int i = 0; i < ways; i++) {
                    if (!sets[index][i].valid) {
                        replace_way = i;
                        break;
                    }
                }

                // eviction
                if (replace_way == -1) {
                    if (config.replacement_policy == "LRU") {
                        long long min_time = -1;
                        for (int i = 0; i < ways; i++) {
                            if (min_time == -1 || sets[index][i].last_access_time < min_time) {
                                min_time = sets[index][i].last_access_time;
                                replace_way = i;
                            }
                        }
                    }
                    else if (config.replacement_policy == "FIFO") {
                        long long min_time = -1;
                        for (int i = 0; i < ways; i++) {
                            if (min_time == -1 || sets[index][i].insertion_time < min_time) {
                                min_time = sets[index][i].insertion_time;
                                replace_way = i;
                            }
                        }
                    }
                    else { // rand
                        replace_way = rand() % ways;
                    }

                    // dirty check and Write-Back
                    if (sets[index][replace_way].dirty && config.write_policy == "WRITE_BACK") {
                        stats.memory_writes++;
                    }
                }

                // insertion of a new block
                sets[index][replace_way].valid = true;
                sets[index][replace_way].tag = tag;
                sets[index][replace_way].dirty = false;
                sets[index][replace_way].last_access_time = global_time;
                sets[index][replace_way].insertion_time = global_time;

                // write to memory if Write-Through on Write-Allocate miss
                if (type == 'W') {
                    if (config.write_policy == "WRITE_THROUGH") {
                        stats.memory_writes++;
                    }
                    else {
                        sets[index][replace_way].dirty = true;
                    }
                }
            }
        }
    }
};

// config parser
void parse_config(string filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        string key = line.substr(0, eq);
        string val = line.substr(eq + 1);

        if (key == "CACHE_SIZE") config.cache_size = stoi(val);
        else if (key == "BLOCK_SIZE") config.block_size = stoi(val);
        else if (key == "ASSOCIATIVITY") config.associativity = stoi(val);
        else if (key == "REPLACEMENT_POLICY") config.replacement_policy = val;
        else if (key == "WRITE_POLICY") config.write_policy = val;
        else if (key == "WRITE_ALLOCATE") config.write_allocate = stoi(val);
    }
}

// main
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: ./cache_sim <config_file> <trace_file>" << endl;
        return 1;
    }

    parse_config(argv[1]);
    CacheSimulator cache(config);

    ifstream trace_file(argv[2]);
    string line;
    char type;
    uint32_t addr;

    while (trace_file >> type >> hex >> addr) {
        cache.access(type, addr);
    }

    // reporting
    cout << "Total loads: " << stats.total_loads << endl;
    cout << "Total stores: " << stats.total_stores << endl;
    cout << "Load hits: " << stats.load_hits << endl;
    cout << "Load misses: " << stats.load_misses << endl;
    cout << "Store hits: " << stats.store_hits << endl;
    cout << "Store misses: " << stats.store_misses << endl;

    double total_accesses = stats.total_loads + stats.total_stores;
    double total_hits = stats.load_hits + stats.store_hits;
    double hit_rate = (total_accesses > 0) ? (total_hits / total_accesses) * 100.0 : 0;

    cout << "Total hit rate: " << fixed << setprecision(2) << hit_rate << "%" << endl;
    cout << "Compulsory misses: " << stats.compulsory_misses << endl;
    cout << "Capacity misses: " << stats.capacity_misses << endl;
    cout << "Conflict misses: " << stats.conflict_misses << endl;
    cout << "Memory reads: " << stats.memory_reads << endl;
    cout << "Memory writes: " << stats.memory_writes << endl;

    double miss_rate = 1.0 - (hit_rate / 100.0);
    double amat = 1 + (miss_rate * 100); // our formula
    cout << "AMAT: " << amat << " cycles" << endl;

    return 0;
}