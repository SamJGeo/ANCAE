#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>

using NodeTag = char;
using Distance = unsigned int;

struct Path {
    NodeTag destination;
    NodeTag nextStep;
    Distance distance;
};

class Graph;

using PathMap = std::unordered_map<NodeTag, Path>;


class Node {
    public:
        Node(NodeTag _id, Graph& g,bool sh,int MAXDEPTH);
        void add_neigh(NodeTag label, Distance dist);
        void update(const PathMap& neighDists, NodeTag neigh);
        bool is_edited() const;
        const NodeTag id;
        void update_neighs();
        void print_status();
        void parse_buffer();
        void set_edited(bool b);
        void send_to_buf(const PathMap& neighDists, NodeTag neigh);
        void disconnect_neigh(NodeTag neighId);

        
    private:
        std::array<std::pair<PathMap, NodeTag>,20> distancesBuffer;
        int bufIndex;
        std::mutex m;
        PathMap distances;
        std::set<NodeTag> neighs;
        bool edited;
        bool sh;
        NodeTag lastEditedBy;
        Graph& graph;
        int MAXDEPTH;
};


class Graph{
    std::unordered_map<NodeTag, std::unique_ptr<Node> > nodes;

    public:
        bool is_settled() const;
        Graph(int numConns, std::vector<std::string> nodeConns, bool sh,std::vector<std::string>nodeFails);
        Node * get_node_by_id(NodeTag nodeID);
        void print_nodes();
        void close_threads();

    private:
        std::vector<std::thread> threads;
};

