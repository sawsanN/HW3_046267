/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <algorithm>
#include <vector>
#include <iostream>

#define Entry -1
#define Exit -2


class Node{

    public:
        InstInfo elem;

        int src1DepInst;
        int src2DepInst;

        int depth;
        int latency;
        //int Instlatency;
        bool entry;
        bool exit;
        int InstId;

        Node(bool Entry_ = false , bool Exit_ = 0){
            elem.opcode = 0;
            elem.dstIdx = 0;
            elem.src1Idx = 0;
            elem.src2Idx = 0;
            src1DepInst = Entry;
            src2DepInst = Entry;
            depth = 0;
            latency = 0;
            entry = Entry_;
            exit = Exit_;
            InstId = -1;
        }
        Node(InstInfo element, int depend1 , int depend2 , int Depth , int latency_ , int id):
            elem(element),src1DepInst(depend1),src2DepInst(depend2),depth(Depth),latency(latency_),entry(false),exit(false),InstId(id){

            }
        
};

bool Set_dependencies(Node* node1 ,Node* node2){


    //unsigned op = node1->elem.opcode;

    //check if there is a true dependency
    if((unsigned)(node1->elem.dstIdx) == node2->elem.src1Idx || (unsigned)(node1->elem.dstIdx) == node2->elem.src2Idx){
       
        if(node1->src1DepInst == Entry )
            node1->src1DepInst = node2->InstId;
        else{
            node1->src2DepInst = node2->InstId;
        }
        return true;
    }

    return false;

}


class Graph{

    public:
        std::vector<Node> InstNodes;


        Node& find_node(int id){
            for(auto& i: InstNodes){
                if(i.InstId == id)
                    return i;
            }
            return InstNodes.back(); // returned exit
        }

        int FindDepth(int id){

            Node& currNode = find_node(id);
            //std::cout<<"inst id is: "<<id<<std::endl; //debug
           // std::cout<<"src2dep is: "<<currNode.src2DepInst<<std::endl; //debug

            if(id==Entry){//node->entry || node == NULL)

                return 0;
            }

            

            int val1 = FindDepth(currNode.src1DepInst);
            int val2 = FindDepth(currNode.src2DepInst);

            return std::max(val1,val2) + currNode.latency  ;

        }

        int find_longest_path(Node& node){

            return FindDepth(node.elem.opcode);
        }
        void push_node(Node node){
            InstNodes.push_back(node);


        }
        int find_graph_depth(){
            return FindDepth(InstNodes.back().elem.opcode);
        }


};

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {


    if(opsLatency == NULL || progTrace == NULL || numOfInsts ==0 )
        return PROG_CTX_NULL;
    
    Graph* OurGraph = new Graph;
    Node entry(true , false);
    OurGraph->push_node(entry);
    for(unsigned i=0 ; i<numOfInsts ;i++){
        Node node(progTrace[i], Entry, Entry , 0, opsLatency[i] , i);
        OurGraph->push_node(node);
    }
    Node exit = Node(false , true);
    exit.src1DepInst = OurGraph->InstNodes[numOfInsts].elem.opcode;
    OurGraph->push_node(exit); // push exit

    for(unsigned i=0 ;i<numOfInsts ;i++){
        Node* node = &(OurGraph->InstNodes[i]);
        for(unsigned j=i+1 ; j<numOfInsts ;j++){
                Set_dependencies(node, &(OurGraph->InstNodes[j]));
            
        }
    }
   // std::cout<<"0 is dependent upon: "<<OurGraph->find_node(1).src1DepInst<<" and: "<<OurGraph->find_node(1).src2DepInst<<std::endl;
    for (auto& i: OurGraph->InstNodes)
        i.depth = OurGraph->find_longest_path(i);

    return OurGraph;
    
}

void freeProgCtx(ProgCtx ctx) {
    free(ctx);
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {

    Graph* OurGraph = static_cast<Graph*>(ctx);
    Node& node = OurGraph->find_node(theInst);
    if(node.exit)
        return -1;

    return OurGraph->find_longest_path(node) ;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {

    Graph* OurGraph = static_cast<Graph*>(ctx);
    Node& node = OurGraph->find_node(theInst);
    if(node.exit)
        return -1;
    *src1DepInst = (OurGraph->find_node(node.src1DepInst).elem.opcode);
    *src2DepInst = (OurGraph->find_node(node.src2DepInst).elem.opcode);

    return 0;
}

int getProgDepth(ProgCtx ctx) {
   
   Graph* OurGraph = static_cast<Graph*>(ctx);
   return  OurGraph->find_graph_depth();
}


