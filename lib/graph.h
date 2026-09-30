/**
 * @file graph.h
 * @author kernkraftwerk (kernkraftdev@hotmail.com)
 * @brief
 * @version 0.1
 * @date 2026-09-29
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef GRAPH_H
#define GRAPH_H
#include "common.h"

/**
 * @brief A vertex consists of the name of the vertex, an unsigned integer, and
 * the color of a vertex
 *
 */
struct GRAPH_VERTEX
{
  uint name;
};

/**
 * @brief An edge consists of  vertex u and a vertex v making up the edge E
 * with E :={u,v}
 *
 */
struct GRAPH_EDGE
{
  struct GRAPH_VERTEX from;
  struct GRAPH_VERTEX to;
};

/**
 * @brief An elegant way to store a graph, each graph consists of a Graph list,
 * which is storing a maximum of 1028 edges, rougly 1KiB in memory
 *
 */
struct GRAPH_EDGE_LIST
{
  struct GRAPH_EDGE edgeList[1028];
  size_t length;
};
#endif
