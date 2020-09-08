#include "graph.hpp"
#include <fstream>

int main(int argc, char * argv[]){


    bool sh = false;


    if (argc == 1)
    {
        std::cout<< "Please include a filename for the network structure\n";
        return 0;
    } else if (argc > 3)
    {
        std::cout << "Too many arguments\n";
        return 0;
    } else if (argc == 3){
        if (std::string(argv[2]).compare("-sh=True") == 0){
            sh=true;
        }else{
            std::cout << "Unfamilar argument, use -sh=True for split horizon enabled. Otherwise split horizon is False" << std::endl;
        }
    }
    
    std::ifstream networkFile(argv[1],std::ifstream::in);



    std::vector<std::string> nodeConns;
    std::vector<std::string> nodeFails;
    if(networkFile.is_open()){
        
        std::string line;

        while (getline(networkFile,line)){
            if (line.size()>4){
                if (line.substr(0,4).compare("fail") == 0){
                    nodeFails.push_back(line.substr(4,line.size()));
                }
            }else{
                nodeConns.push_back(line);
            }
        }


    }else{
        std::cout << "File Failed to open, filename may be invalid\n";
        return 0;
    }

    int numConns = std::stoi(nodeConns.at(0));
    nodeConns.erase(nodeConns.begin());

    // std::cout << "Number of nodes:" << numConns << "\n";
    // for(std::string s: nodeConns){
    //     std::cout << s << "\n";
    // }

    // for(std::string s: nodeFails){
    //     std::cout << s;
    // }


    Graph g = Graph(numConns,nodeConns,sh,nodeFails);
}