#include "graph.hpp"


Node::Node(NodeTag _id, Graph &g, bool _sh, int _MAXDEPTH) : id(_id), graph(g) , sh(_sh), MAXDEPTH(_MAXDEPTH)
{
    //set up map to contain an entry for itself with a distance of zero
    Path dest = {_id,_id,0};
    distances.emplace(_id,dest);
    std::mutex m;
    bufIndex = 0;

};


bool Node::is_edited() const {
    return edited;
};


void Node::print_status() {

    // Destination distance nextstep lastupdatedby 
    std::cout << "Node:" << id << "   Edited:" << edited << "\n";
    for (auto [dest_id,dest]: distances){
        std::cout << "Destination ID:" << dest_id << "   Distance:" << dest.distance << "   Next Step:" << dest.nextStep << "\n";
    }

}


void Node::add_neigh(NodeTag _label,Distance _d){
    neighs.insert(_label);
    Path dest = {_label,_label,_d};
    distances.emplace(_label,dest);
};
int updatecount = 0;

void Node::update_neighs(){
    for(NodeTag n: neighs){
        graph.get_node_by_id(n)->send_to_buf(distances,id);
    }
};

void Node::parse_buffer(){
    //Once buffers are up to date, update the internal distances, first come first serve
    for(int i = 0; i<bufIndex;i++){
        update(distancesBuffer[i].first,distancesBuffer[i].second);
    }

    //reset buffer index (means we will overwrite data on next run)
    bufIndex = 0;

};

void Node::set_edited(bool b){
    edited = b;
};

void Node::send_to_buf(const PathMap& neighDists, NodeTag neigh){
    std::unique_lock<std::mutex>(m);

    if(bufIndex == 20){
        std::cout << "Buffer on node " << id << " is full\n";
    } else{
        distancesBuffer[bufIndex] = {neighDists,neigh};
        bufIndex++;
    }
};

void Node::disconnect_neigh(NodeTag neighId){

    //in the event of a disconnnect then we can call this function to notify a node
    if(neighs.find(neighId)!= neighs.end()){
        //remove entry from distance and set edited to true
        distances.erase(neighId);
        neighs.erase(neighId);
        set_edited(true);

    }else{
        std::cout << "Failure to disconnect node: " << id << " and node: " << neighId << " as nodes are not neighbours" << std::endl;
    }

};

void Node::update(const PathMap &neighDists,NodeTag neigh){

    Distance distToNeigh = distances[neigh].distance;
    NodeTag stepToNeigh = distances[neigh].nextStep;

    std::vector<NodeTag> neighsCovered;
    // Iterate over the map using iterator
	for (auto& [dest, neighInfo] : neighDists)
	{
        //dont bother updating table if the destination is yourself
        //also adds the split horizon, it wont consider cases where split horizon is true and
        //next step is through the node
        if (dest != id && !(sh == true && neighInfo.nextStep == id)){
            auto curPathInfo = distances.find(dest);


            //calculate the new distance based on the distance to the neigh plus the neighs distance to the destination
            unsigned int distToDest = distToNeigh + neighInfo.distance;

            //if we have an entry for the destination
            if (curPathInfo != distances.end())
            {
                //if the path is better or the path and theyre not over the maxdepth
                if (distToDest < curPathInfo->second.distance && !(distToDest >= MAXDEPTH && curPathInfo->second.distance >= MAXDEPTH)){
                    //change the data for the current node as new node has better distance
                    curPathInfo->second.distance = distToDest;
                    curPathInfo->second.nextStep = stepToNeigh;
                    set_edited(true);
                //if the node advertising is the one we get our route from normally then update
                } else if ((curPathInfo->second.nextStep == neigh && !(distToDest == curPathInfo->second.distance)) && !(distToDest >= MAXDEPTH && curPathInfo->second.distance >= MAXDEPTH)) {
                    curPathInfo->second.distance = distToDest;
                    set_edited(true);
                } else if (distToDest >= MAXDEPTH && curPathInfo->second.distance >= MAXDEPTH){
                    curPathInfo->second.distance = MAXDEPTH;
                }

            //path didnt exist before so add it to the table and set edited to true
            }else{
                Path _p = {dest,stepToNeigh,distToDest};
                distances.emplace(dest,_p);
                set_edited(true);
            }

        }
	}


    

};
