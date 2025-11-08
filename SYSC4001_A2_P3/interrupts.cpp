#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

/* ------------------------- small utils ------------------------- */
static inline string trim(string s){
    auto notsp = [](int c){ return !isspace(c); };
    s.erase(s.begin(), find_if(s.begin(), s.end(), notsp));
    s.erase(find_if(s.rbegin(), s.rend(), notsp).base(), s.end());
    return s;
}

// normalize "CPU, 10" -> "CPU,10"
static inline string normalize_commas(const string& s){
    string out; out.reserve(s.size());
    for(size_t i=0;i<s.size();++i){
        if(s[i]==',' && i+1<s.size() && s[i+1]==' ') { out.push_back(','); ++i; }
        else out.push_back(s[i]);
    }
    return out;
}

/* ------------------------- loaders ------------------------- */
// vector_table: "<vector_number> <address>"
static bool load_vector_table(const string& path, vector<string>& tab){
    ifstream in(path);
    if(!in.is_open()) return false;
    vector<pair<int,string>> rows;
    int idx; string addr;
    while(in >> idx >> addr){
        rows.emplace_back(idx, addr);
    }
    if(rows.empty()) return false;
    int mx = 0;
    for(auto& r: rows) mx = max(mx, r.first);
    tab.assign(mx+1, "");
    for(auto& r: rows) tab[r.first] = r.second;
    return true;
}

// device_table: "<device_id> <delay_ms>"
static bool load_device_table(const string& path, vector<int>& delays){
    ifstream in(path);
    if(!in.is_open()) return false;
    vector<pair<int,int>> rows;
    int dev, d;
    while(in >> dev >> d){
        rows.emplace_back(dev, d);
    }
    if(rows.empty()) return false;
    int mx = 0;
    for(auto& r: rows) mx = max(mx, r.first);
    delays.assign(mx+1, 0);
    for(auto& r: rows) delays[r.first] = r.second;
    return true;
}

// external_files: "<program_name> <size_mb>"
static bool load_external_sizes(const string& path, unordered_map<string,int>& sizes){
    ifstream in(path);
    if(!in.is_open()) return false;
    string name; int sz;
    while(in >> name >> sz){
        sizes[name] = sz;
    }
    return !sizes.empty();
}

// generic line loader (trace + program traces)
static bool load_lines(const string& path, vector<string>& out){
    ifstream in(path);
    if(!in.is_open()) return false;
    string line;
    while(getline(in, line)){
        line = trim(line);
        if(!line.empty()) out.push_back(line);
    }
    return !out.empty();
}

/* ------------------------- parsing ------------------------- */
// Return (activity, numeric_arg, program)
// Accepts:
//   "CPU, 100"         -> ("CPU",100,"")
//   "SYSCALL, 4"       -> ("SYSCALL",4,"")
//   "END_IO, 4"        -> ("END_IO",4,"")
//   "ENDIO, 4"         -> ("END_IO",4,"")
//   "FORK, 10"         -> ("FORK",10,"")
//   "EXEC program1,50" -> ("EXEC",50,"program1")
static tuple<string,int,string> parse_trace_line(const string& raw_in){
    string s = normalize_commas(trim(raw_in));
    // split once by comma
    size_t p = s.find(',');
    if(p == string::npos){
        // fallback: maybe "CPU 100" or "EXEC program 50"
        string a,b; int n=0;
        stringstream ss(s);
        ss >> a;
        if(a == "EXEC"){
            ss >> b >> n;
            return {string("EXEC"), n, trim(b)};
        } else {
            ss >> n;
            return {a, n, string()};
        }
    }
    string left = trim(s.substr(0,p));
    int num = stoi(trim(s.substr(p+1)));

    // left can be "CPU" or "SYSCALL" or "EXEC <prog>" etc.
    string head, rest;
    {
        stringstream ss(left);
        ss >> head;
        getline(ss, rest);
        rest = trim(rest);
    }

    if(head == "ENDIO") head = "END_IO";
    if(head == "EXEC")  return {string("EXEC"), num, rest};
    return {head, num, string()};
}

/* ------------------------- ISR boilerplate ------------------------- */
// 10 time units: 1 switch, 1 save, 1 load-vector, 7 jump
static pair<string,int> isr_boiler(int t, int vec, const vector<string>& vectors){
    string out;
    out += to_string(t) + ", 1, switch to kernel mode\n"; ++t;
    out += to_string(t) + ", 1, save context\n";          ++t;
    string addr = (0 <= vec && vec < (int)vectors.size() && !vectors[vec].empty())
                  ? vectors[vec] : string("0x????");
    out += to_string(t) + ", 1, load vector " + to_string(vec) + " (" + addr + ")\n"; ++t;
    out += to_string(t) + ", 7, jump to ISR\n";           t += 7;
    return {out, t};
}

/* ------------------------- status helpers ------------------------- */
static void status_header(string& buf, int t, const string& tag){
    buf += "time: " + to_string(t) + "; current trace: " + tag + "\n";
    buf += "+------------------------------------------------------+\n";
    buf += "| PID | program name | partition number | size | state |\n";
    buf += "+------------------------------------------------------+\n";
}
static void status_row(string& buf, const string& state="running"){
    buf += "|  ?  |      ?       |        ?         |  ?   | " + state + " |\n";
}
static void status_footer(string& buf){ buf += "+------------------------------------------------------+\n"; }

static void snapshot(string& status, int t, const string& tag){
    status_header(status, t, tag);
    status_row(status, "running");
    status_footer(status);
}

/* ------------------------- core engine ------------------------- */
static tuple<string,string,int>
run_trace(const vector<string>& lines,
          int start_time,
          const vector<string>& vectors,
          const vector<int>& delays,
          const unordered_map<string,int>& sizes)
{
    string exec, status;
    int t = start_time;

    snapshot(status, t, "START");

    for(size_t i=0;i<lines.size();++i){
        auto [act, num, prog] = parse_trace_line(lines[i]);

        if(act == "CPU"){
            exec += to_string(t) + ", " + to_string(num) + ", CPU Burst\n";
            t += num;
            snapshot(status, t, "CPU," + to_string(num));
        }
        else if(act == "SYSCALL"){
            // vector = device number
            int vec = (num >= 0 && num < (int)vectors.size()) ? num : 0;
            auto [pre,t2] = isr_boiler(t, vec, vectors); exec += pre; t = t2;

            int dly = (num >= 0 && num < (int)delays.size()) ? delays[num] : 0;
            exec += to_string(t) + ", " + to_string(dly) + ", SYSCALL ISR\n"; t += dly;
            exec += to_string(t) + ", 1, IRET\n"; ++t;

            snapshot(status, t, "SYSCALL," + to_string(num));
        }
        else if(act == "END_IO" || act == "ENDIO"){
            // vector = device number
            int vec = (num >= 0 && num < (int)vectors.size()) ? num : 0;
            auto [pre,t2] = isr_boiler(t, vec, vectors); exec += pre; t = t2;

            int dly = (num >= 0 && num < (int)delays.size()) ? delays[num] : 0;
            exec += to_string(t) + ", " + to_string(dly) + ", ENDIO ISR\n"; t += dly;
            exec += to_string(t) + ", 1, IRET\n"; ++t;

            snapshot(status, t, "END_IO," + to_string(num));
        }
        else if(act == "FORK"){
            // Use vector 2 for FORK (adjust if your table says otherwise)
            auto [pre,t2] = isr_boiler(t, 2, vectors); exec += pre; t = t2;
            exec += to_string(t) + ", 2, FORK ISR\n"; t += 2;
            exec += to_string(t) + ", 1, IRET\n"; ++t;

            // collect child block between IF_CHILD ... IF_PARENT/ENDIF
            vector<string> child;
            bool child_phase = false;
            size_t parent_pos = i;
            for(size_t j=i+1;j<lines.size();++j){
                auto [a2,_,__] = parse_trace_line(lines[j]);
                if(a2 == "IF_CHILD"){ child_phase = true; continue; }
                if(a2 == "IF_PARENT"){ child_phase = false; parent_pos = j; break; }
                if(a2 == "ENDIF"){ break; }
                if(child_phase) child.push_back(lines[j]);
            }
            i = parent_pos;

            // run child first
            if(!child.empty()){
                auto [ce, cs, nt] = run_trace(child, t, vectors, delays, sizes);
                exec += ce; status += cs; t = nt;
            }
            snapshot(status, t, "FORK");
        }
        else if(act == "EXEC"){
            // Use vector 3 for EXEC (adjust if your table says otherwise)
            auto [pre,t2] = isr_boiler(t, 3, vectors); exec += pre; t = t2;

            exec += to_string(t) + ", 3, EXEC ISR\n"; t += 3;
            exec += to_string(t) + ", 1, IRET\n"; ++t;

            // loader time = size(MB) * 15
            int mb = 0; auto it = sizes.find(prog); if(it != sizes.end()) mb = it->second;
            int load_ms = (mb > 0 ? mb * 15 : 0);
            exec += to_string(t) + ", " + to_string(load_ms) + ", LOAD PROGRAM " + prog + "\n"; t += load_ms;

            // run external program trace
            vector<string> sub;
            load_lines(prog + ".txt", sub); // safe if missing -> sub empty, no-op
            if(!sub.empty()){
                auto [e2, s2, t2b] = run_trace(sub, t, vectors, delays, sizes);
                exec += e2; status += s2; t = t2b;
            }

            // parent stops after exec (replaced image)
            snapshot(status, t, "EXEC " + prog);
            break;
        }
        // IF_CHILD / IF_PARENT / ENDIF are structural markers handled in FORK
    }

    return {exec, status, t};
}

/* ------------------------- main ------------------------- */
int main(int argc, char** argv){
    if(argc < 5){
        fprintf(stderr, "ERR: Expected 4 args: <trace> <vector_table> <device_table> <external_files>\n");
        return 1;
    }

    string trace_path = argv[1];
    string vec_path   = argv[2];
    string dev_path   = argv[3];
    string ext_path   = argv[4];

    vector<string> vectors;
    vector<int> delays;
    unordered_map<string,int> sizes;
    vector<string> trace;

    if(!load_vector_table(vec_path, vectors)){
        fprintf(stderr, "ERR: could not load vector_table\n");
        return 1;
    }
    if(!load_device_table(dev_path, delays)){
        fprintf(stderr, "ERR: could not load device_table\n");
        return 1;
    }
    if(!load_external_sizes(ext_path, sizes)){
        fprintf(stderr, "ERR: could not load external_files\n");
        return 1;
    }
    if(!load_lines(trace_path, trace)){
        fprintf(stderr, "ERR: could not load trace\n");
        return 1;
    }

    auto [exec, sys, _tend] = run_trace(trace, 0, vectors, delays, sizes);

    ofstream("execution.txt")     << exec;
    ofstream("system_status.txt") << sys;

    return 0;
}
