
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
            status = true; //table has changed
            break;
          }
        }
        else
        {
          status = true; //table has changed
          break;
        }
      }
    }
    if (!status)
      break;
  }
  printRT(nd);

  //change routing table entries for nodes B and C

  for (int i = 0; i < nd.size(); i++)
  {
    if (nd[i]->getName() == "B")
    {
      nd[i]->updateTblEntry("10.0.1.23", "10.0.1.3");
    }
    else if (nd[i]->getName() == "C")
    {
      nd[i]->updateTblEntry("10.0.1.3", "10.0.0.21");
      nd[i]->updateTblEntry("10.0.1.3", "10.0.1.23");
    }
  }

  cout << "\nAfter changing max the routing table entries for node B and C to max hop count 16, the new routing table with poison reverse:\n";
  /*Since the link between B and C has been removed, (as confirmed by ma'am), C's routing table won't be updated anymore.*/

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
            status = true; //table has changed
            break;
          }
        }
        else
        {
          status = true; //table has changed
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
          mytbl.tbl[j].cost = min(msg->mytbl->tbl[i].cost + 1, 16);
        }
        else if (msg->mytbl->tbl[i].cost + 1 < mytbl.tbl[j].cost)
        {
          mytbl.tbl[j].ip_interface = msg->from;
          mytbl.tbl[j].nexthop = msg->mytbl->tbl[i].ip_interface;
          mytbl.tbl[j].cost = min(msg->mytbl->tbl[i].cost + 1, 16);
        }
      }
    }
    if (!flag)
    {
      RoutingEntry newentry;
      newentry.ip_interface = msg->recvip;
      newentry.dstip = msg->mytbl->tbl[i].dstip;
      newentry.nexthop = msg->from;
      msg->from.compare(newentry.dstip) == 0 ? newentry.cost = 1 : newentry.cost = min(msg->mytbl->tbl[i].cost + 1, 16);
      mytbl.tbl.push_back(newentry);
    }
  }
}