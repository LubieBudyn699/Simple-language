#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    // Ensure the user provided an input file argument
    if (argc < 2) {
        cout << "Error: No input file specified." << endl;
        cout << "Usage: " << argv[0] << " <filename.simple>" << endl;
        return 1;
    }

    string input_filename = argv[1];
    ifstream source_file(input_filename);

    // Verify the source file exists and can be opened
    if (!source_file.is_open()) {
        cout << "Error: Could not open source file '" << input_filename << "'" << endl;
        return 1;
    }

    // Create a temporary C++ file that will hold the transpiled source code
    ofstream plik_cpp("temp_output.cpp");
    if (!plik_cpp.is_open()) {
        cout << "Internal Error: Could not create compilation cache." << endl;
        return 1;
    }
    
    // Inject mandatory C++ entry point headers
    plik_cpp << "#include <iostream>\n";
    plik_cpp << "#include <cstdlib>\n"; // Required for executing low-level std::system shell calls
    plik_cpp << "int main() {\n";

    string linia_kodu;
    int line_number = 0;

    // Read and parse the .simple file line by line
    while (getline(source_file, linia_kodu)) {
        line_number++;

        // Skip completely empty lines or basic whitespace modifications
        if (linia_kodu.empty() || linia_kodu == "\n" || linia_kodu == "\r") {
            continue;
        }

        // === PARSER: Lexical validation for 'say' token ===
        if (linia_kodu.find("say(\"") == 0) {
            size_t start = 5; // Skip the length of 'say("' string prefix
            size_t koniec = linia_kodu.find("\")");
            
            if (koniec != string::npos) {
                string tekst_w_srodku = linia_kodu.substr(start, koniec - start);
                plik_cpp << "    std::cout << \"" << tekst_w_srodku << "\" << std::endl;\n";
            } else {
                cout << "SYNTAX ERROR [" << input_filename << ":" << line_number << "]: Missing closing parenthesis ')' for 'say' command." << endl;
                plik_cpp.close();
                system("rm -f temp_output.cpp");
                return 1;
            }
        } 
        // === PARSER: Lexical validation for 'execute' token ===
        else if (linia_kodu.find("execute(\"") == 0) {
            size_t start = 9; // Skip the length of 'execute("' string prefix
            size_t koniec = linia_kodu.find("\")");
            
            if (koniec != string::npos) {
                string tekst_w_srodku = linia_kodu.substr(start, koniec - start);
                plik_cpp << "    std::system(\"" << tekst_w_srodku << "\");\n";
            } else {
                cout << "SYNTAX ERROR [" << input_filename << ":" << line_number << "]: Missing closing parenthesis ')' for 'execute' command." << endl;
                plik_cpp.close();
                system("rm -f temp_output.cpp");
                return 1;
            }
        }
        // === PARSER fallback: Unknown command definition ===
        else {
            cout << "SYNTAX ERROR [" << input_filename << ":" << line_number << "]: Unknown identifier/command -> '" << linia_kodu << "'" << endl;
            plik_cpp.close();
            system("rm -f temp_output.cpp");
            return 1;
        }
    }

    // Seal the C++ main function wrapper
    plik_cpp << "    return 0;\n";
    plik_cpp << "}\n";
    plik_cpp.close();
    source_file.close();

    cout << "[scc] Transpilation successful. Invoking backend g++ compiler..." << endl;
    
    // Trigger native g++ compilation using standard aggressive performance optimizations (-O3)
    int compile_status = system("g++ -O3 temp_output.cpp -o program_simple");
    
    // Purge the transient build architecture cache immediately
    system("rm -f temp_output.cpp");

    if (compile_status == 0) {
        cout << "[scc] Compilation complete. Native binary produced: './program_simple'" << endl;
    } else {
        cout << "[scc] Internal Error: Backend binary assembly failed during g++ invocation." << endl;
        return 1;
    }

    return 0;
}
