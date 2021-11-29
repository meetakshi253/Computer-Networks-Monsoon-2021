
#include "node.h"
#include <iostream>

using namespace std;

void printRT(vector<RoutingNode *> nd)
{
  /*Print routing table entries*/
  for (int i = 0; i < nd.size(); i++)
  {
    nd[i]->printTable();
  }
}

void routingAlgo(vector<RoutingNode *> nd)
{
  vector<struct routingtbl> prev; //to store the previous routing table
  bool status;                    //to check if routing table has changed. false==not changed
  while (1)
  {
    prev.clear();
    status = false;
    for (int i = 0; i < nd.size(); ++i)
    {
      prev.push_back(nd[i]->getTable());
    }
    for (int i = 0; i < nd.size(); ++i)
    {
      nd[i]->sendMsg(); //send message to other nodes
    }
    /*check if the routing tables have converged*/
    for (int i = 0; i < nd.size(); i++)
    {
      if (nd[i]->getTable().tbl.size() != prev[i].tbl.size())
      {
        status = true;
        break;
      }
      for (int j = 0; j < nd[i]->getTable().tbl.size(); j++)
      {
        if (nd[i]->getTable().tbl[j].nexthop.compare(prev[i].tbl[j].nexthop) == 0)
        {
          if (nd[i]->getTable().tbl[j].ip_interface.compare(prev[i].tbl[j].ip_interface) == 0 && nd[i]->getTable().tbl[j].cost == prev[i].tbl[j].cost)
          {
            continue;
          }
          else
          {
            status = true;
            break;
          }
        }
        else
        {
          status = true;
          break;
        }
      }
    }
    if (!status)
      break;
  }
  printRT(nd);
}

void RoutingNode::recvMsg(RouteMsg *msg)
{
  bool flag; //entry exists or not
  for (int i = 0; i < msg->mytbl->tbl.size(); i++)
  {
    flag = false;
    for (int j = 0; j < mytbl.tbl.size(); j++)
    {
      if (msg->mytbl->tbl[i].dstip.compare(mytbl.tbl[j].dstip) == 0)
      {
        flag = true;
        if (mytbl.tbl[j].nexthop.compare(msg->from) == 0)
        {
          mytbl.tbl[j].cost = msg->mytbl->tbl[i].cost + 1;
        }
        else if (msg->mytbl->tbl[i].cost + 1 < mytbl.tbl[j].cost)
        {
          mytbl.tbl[j].ip_interface = msg->from;
          mytbl.tbl[j].nexthop = msg->mytbl->tbl[i].ip_interface;
          mytbl.tbl[j].cost = msg->mytbl->tbl[i].cost + 1;
        }
      }
    }
    if (!flag)
    {
      RoutingEntry newentry;
      newentry.ip_interface = msg->recvip;
      newentry.dstip = msg->mytbl->tbl[i].dstip;
      newentry.nexthop = msg->from;
      msg->from.compare(newentry.dstip) == 0 ? newentry.cost = 1 : newentry.cost = msg->mytbl->tbl[i].cost + 1;
      mytbl.tbl.push_back(newentry);
    }
  }
}
