#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include "CompilerHeaders.h"

int main(int argc, char *argv[]) {
    // In C/C++ argc is 1 if nothing is passed because argv[0] contains the program name
    if (argc == 2) {
        Parser parser;
        std::string path = argv[1];
        struct stat status;

        // Check if its a valid path
        if (stat(argv[1], &status) == 0) {
            if (status.st_mode & S_IFDIR) { // If its a directory
                DIR *dir;
                struct dirent *jackFile;
                if ((dir = opendir(argv[1])) != nullptr) {
                    parser.AddJackOS();
                    while ((jackFile = readdir(dir)) != nullptr) {
                        std::string filename = jackFile->d_name;
                        if (filename.substr(filename.find_last_of(".") + 1) == "jack") {
                            // Extract the filename without the extension
                            std::string withoutExtension;
                            withoutExtension = filename.substr(0, filename.find_last_of("."));

                            // Create VmFile object and add it to the list
                            Parser::VmFile file;
                            file.filename = withoutExtension;
                            parser.vmFiles.push_back(file);

                            std::string filePath = path + '/' + filename;
                            bool init = parser.Init(filePath);
                            if (init) {
                                parser.ClassDeclar();
                            }
                        }
                    }
                    closedir(dir);
                }
                else { // Could not open directory
                    std::cout << "Couldn't open directory " << path << std::endl;
                    exit(0);
                }
            }
            else if (status.st_mode & S_IFREG) { // If its a file
                // Check that its a JACK source file
                if (path.substr(path.find_last_of(".") + 1) == "jack") {
                    parser.AddJackOS();
                    // Extract the filename without the extension
                    std::string filename;
                    size_t start = path.rfind('/', path.length());
                    size_t end = path.rfind('.', path.length());
                    filename = path.substr(start+1, path.length()-end-1);

                    // Create VMFile object and add it to the list
                    Parser::VmFile file;
                    file.filename = filename;
                    parser.vmFiles.push_back(file);

                    bool init = parser.Init(path);
                    if (init) {
                        parser.ClassDeclar();
                    }

                    // For outputting the file adjust the path
                    path = path.substr(0, path.find_last_of('/'));
                }
                else
                    std::cout << path << " is not a JACK source file." << std::endl;
            }
        }
        else
            std::cout << "No such file or directory " << path << std::endl;

        // Once all the parsing is done resolve everything and check
        parser.ResolveAllDeclars();

        // Both the compilation and checks are complete, write the VM files now
        parser.WriteVmFiles(path);
    }
    else
        std::cout << "Please pass only one JACK file or folder path." << std::endl;

    return 0;
}
