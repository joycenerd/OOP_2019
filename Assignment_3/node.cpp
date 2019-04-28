#include "node.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <cstring>
using namespace std;


void Node::printNeighbor(){
    int vsize=0,i;
    vsize=planarGraph.size();
    printf("[%d] ",id);
    for(i=0;i<vsize;i++){
        printf("%d ",planarGraph[i].id);
    }
    printf("\n");
}

void Node::planarize(){
    int vsize,i,j,yn;
    double midX,midY,radius,distance;
    vsize=beforeNeighbor.size();
    for(i=0;i<vsize;i++){
        midX=(this->x+beforeNeighbor[i].x)/2;
        midY=(this->y+beforeNeighbor[i].y)/2;
        radius=sqrt(pow(midX-this->x,2)+pow(midY-this->y,2));
        yn=1;
        for(j=0;j<vsize;j++){
            if(j==i) continue;
            distance=sqrt(pow(midX-beforeNeighbor[j].x,2)+pow(midY-beforeNeighbor[j].y,2));
            if(distance<radius){
                yn=0;
                break;
            }
        }
        if(yn) planarGraph.push_back(beforeNeighbor[i]);
    }
}

void Node::addDst(int inDstId,double inDstX,double inDstY){
    dstId=inDstId;
    dstX=inDstX;
    dstY=inDstY;
}

int Node::checkQueue(int mode){
    Packet packet;
    int nextId;
    if(q_pkt.empty()) return -1;
    packet=q_pkt.front();
    nextId=packet.getNxtId();
    if(mode==1) printf("%d\n",nextId);
    if(nextId==id) return id;
    q_pkt.pop();
    return -1;
}

double calcSmallestAngle(double curX,double curY,double prevX,double prevY,double nxtX,double nxtY){
    double aX,aY,bX,bY,aAbs,bAbs,degree;
    aX=prevX-curX;
    aY=prevY-curY;
    bX=nxtX-curX;
    bY=nxtY-curY;
    aAbs=sqrt(aX*aX+aY*aY);
    bAbs=sqrt(bX*bX+bY*bY);
    degree=acos((aX*bX+aY*bY)/(aAbs*bAbs));
    return degree;
}

bool sortBySec(const pair<int,double> &a,const pair<int,double> &b){
    return a.second<b.second;
}

double lineFunc(double curX,double curY,double lastX,double lastY){
    double slope;
    slope=(lastY-curY)/(lastX-curX);
    return slope;
}

void getNewItx(double slope1,double x1,double y1,double slope2,double x2,double y2,double &newItxX,double &newitxY){
    newItxX=(-slope2*x2+y2+slope1*x1-y1)/(slope1-slope2);
    newitxY=slope1*(newItxX-x1)+y1;
}

void Node::getNextHop(vector<Node> &v_nodes){
    Packet packet;
    double itxX,itxY,slope,side,neighborX,neighborY,degree,lastX,lastY,sdSlope;
    double curSide,neighborSide,isSame,newItxX,newItxY;
    vector<pair<int,double>> angle;
    int i,vsize,neighborId,dstId,lastId;
    int vis[1010];
    packet=q_pkt.front();
    itxX=packet.getItxX();
    itxY=packet.getItxY();
    sdSlope=lineFunc(x,y,dstX,dstY);
    vsize=planarGraph.size();
    if(x==itxX && y==itxY){
        for(i=0;i<vsize;i++){
            neighborId=planarGraph[i].id;
            neighborX=planarGraph[i].x;
            neighborY=planarGraph[i].y;
            side=sdSlope*(neighborX-x)-(neighborY-y);
            degree=calcSmallestAngle(x,y,dstX,dstY,neighborX,neighborY);
            if((side>=0 && dstX>x) || (side<=0 && dstX<x)) angle.push_back(make_pair(neighborId,degree));
            else angle.push_back(make_pair(neighborId,2*M_PI-degree));
        }
        sort(angle.begin(),angle.end(),sortBySec);
        q_pkt.pop();
        packet.modifyInfo(id,angle[0].first);
        q_pkt.push(packet);
        return;
    }
    lastId=packet.getLastId();
    lastX=v_nodes[lastId].x;
    lastY=v_nodes[lastId].y;
    memset(vis,0,sizeof(vis));
    while(1){
        slope=lineFunc(x,y,lastX,lastY);
        for(i=0;i<vsize;i++){
            neighborId=planarGraph[i].id;
            neighborX=planarGraph[i].x;
            neighborY=planarGraph[i].y;
            side=slope*(neighborX-x)-(neighborY-y);
            degree=calcSmallestAngle(x,y,lastX,lastY,neighborX,neighborY);
            if((side>=0 && dstX>x) || side<=0 && dstX<x) angle.push_back(make_pair(neighborId,degree));
            else angle.push_back(make_pair(neighborId,2*M_PI-degree)); 
        }
        sort(angle.begin(),angle.end(),sortBySec);
        neighborId=angle[0].first;
        neighborX=v_nodes[neighborId].x;
        neighborY=v_nodes[neighborId].y;
        curSide=sdSlope*(x-dstX)-(y-dstY);
        neighborSide=sdSlope*(neighborX-dstX)-(neighborY-dstY);
        isSame=curSide*neighborSide;
        if(isSame>=0){
             q_pkt.pop();
             packet.modifyInfo(id,neighborId);
            q_pkt.push(packet);
            return;
        }
        slope=lineFunc(neighborX,neighborY,x,y);
        getNewItx(sdSlope,dstX,dstY,slope,x,y,newItxX,newItxY);
        if((dstX>x && newItxX<=itxX && newItxY<=itxY) || (dstX<x && newItxX>=itxX && newItxY>=itxY)){
            q_pkt.pop();
             packet.modifyInfo(id,neighborId);
            q_pkt.push(packet);
            return;
        }
        
    }
}