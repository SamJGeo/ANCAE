#include "graph.hpp"
#include <algorithm>
#include <functional>

Graph::Graph(int numConns, std::vector<std::string> nodeConns, bool sh,std::vector<std::string>nodeFails) {
    
    //option 1 - simple network
    /*

       2
     a---b
    6 \ / 3
       c
       | 1
       d

    */


   std::vector<NodeTag> nodeTags;
   //maxdepth will be one above the sum of all links in the network, although it is normally sixteen hops
   int maxdepth = 1;
       for (int i =0 ;i<numConns;i++){

       NodeTag first = nodeConns[i][0];
       NodeTag second = nodeConns[i][1];
       int distance = std::stoi(nodeConns[i].substr(2,nodeConns[i].size()));
       maxdepth = maxdepth + distance;
    }


   for (int i =0 ;i<numConns;i++){

       NodeTag first = nodeConns[i][0];
       NodeTag second = nodeConns[i][1];
       int distance = std::stoi(nodeConns[i].substr(2,nodeConns[i].size()));
       

       //Check if the nodes in the link already exist, if not, create them
       if (std::find(nodeTags.begin(),nodeTags.end(),first) == nodeTags.end()){
           std::cout << "Creating node " << first << "\n";
           nodes.insert({first, std::make_unique<Node>(first, *this, sh, maxdepth)});
           nodeTags.push_back(first);
       }
       if (std::find(nodeTags.begin(),nodeTags.end(),second) == nodeTags.end()){
           std::cout << "Creating node " << second << "\n";
           nodes.insert({second, std::make_unique<Node>(second, *this, sh, maxdepth)});
           nodeTags.push_back(second);
       }

       //build connection
        nodes[first]->add_neigh(second,distance);
        nodes[second]->add_neigh(first,distance);

   }

    std::cout << "------NETWORK START STATE------" << std::endl;
    print_nodes();
    std::cout << "-------------------------------" << std::endl;

    int numSteps = 0;
    do
    {
        //set edited for each node to false
        for (const auto& [key,node] : nodes){
            node->set_edited(false);
        }

        for(std::string f: nodeFails){
            char disFrom = f.at(1);
            char disTo = f.at(2);
            int stepActed = f.at(0) - '0';

            if(stepActed == numSteps){
                nodes[disFrom]->disconnect_neigh(disTo);
                std::cout << "Disconnecting node:" << disTo << " from:" << disFrom << " at step:" << numSteps << std::endl;
                nodeTags.erase(std::find(nodeTags.begin(),nodeTags.end(),disTo));
                nodes.erase(disTo);
                print_nodes();
            }
        }


        // open a thread for each node
        // run update
        for(const NodeTag& n: nodeTags){

            auto updateneighcallback = std::bind(&Node::update_neighs, nodes[n].get());
            threads.emplace_back(updateneighcallback);
        }

        close_threads();

        for(auto n : nodeTags){
            auto parsebuffercallback = std::bind(&Node::parse_buffer, nodes[n].get());
            threads.emplace_back(parsebuffercallback);
        }

        close_threads();

        //if the graph isnt settled then steps is +1 
        if (!is_settled()){
            numSteps++;
            std::cout << "Step Count: " << numSteps << std::endl;
            print_nodes();
        }else{
            std::cout << "------------STABLE------------" << std::endl;
            std::cout << "Network Stable in " << numSteps << " number of steps" << std::endl;
            print_nodes();
        }

        

    } while (!is_settled());


    // nodes['b']->update_neighs();

    // std::cout << "\n\n\n";


    // for (const auto& [key,node] : nodes){
    //     node->print_status();
    // }

}

void Graph::close_threads(){
    for (std::thread & t_: threads){
        // std::cout << "Deleting thread\n";
        if (t_.joinable()){
            t_.join();
        } else{
            std::cout << "Thread not joinable\n";
        }
    }
    //Clear the vector
    threads.clear();
}



Node * Graph::get_node_by_id(NodeTag nodeID){
    return(nodes[nodeID].get());
}

void Graph::print_nodes(){
    std::cout << std::endl;
    for (const auto& [key,node] : nodes){
        node->print_status();
    }
    std::cout << std::endl;
}


bool Graph::is_settled() const{
    bool s = true;
    for(const auto& [key,node] : nodes){
        if(node->is_edited())
        {s=false;}
    }
    return s;
}