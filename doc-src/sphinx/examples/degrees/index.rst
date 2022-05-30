.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

Six Degrees of Separation
=========================

To illustrate the effectiveness of NWGraph’s "there is no graph" design
philosophy, this section walks through an example that ingests a
database table and uses graph algorithms for some basic knowledge
discovery. The desired goal is to the find the popularized Bacon number,
which is the degree of separation of various Hollywood actors from the
actor Kevin Bacon. The starting point of this example is an Oracle web
crawl of Wikipedia which is then parsed by an open source script to
create a Wikipedia movie database, with several entries of the following
form.

.. code:: js

   {
     "title": "Movie Title",
     "cast": "Actor1","Actor2",
     "directors": "Director",
     ...
     "year": year
   }

This is not a graph; it is a relational database that we wish to query
with graph algorithms. After reading into a vector of json entries, we
will populate an NWGraph edge list with directed edges from movie titles
to actors. We will also keep a map from movie titles to movie ids, and
from actor names to actor ids.

.. code:: c++

   vector<json> jsons;
   // Populate jsons vector

   map<string, size_t> titles_map, names_map;
   vector<string>      titles, names;

   nw::graph::edge_list<nw::graph::directed> edges;

   for (auto& j: jsons) {
     auto title = j["title"]
     if (titles_map.find(title) == titles_map.end()) {
       // Add title to title map if it
       // doesn't exist
     }
     
     // Movie has multiple cast members
     for (auto& k : j["cast"]) {
       auto name = delink(k);
       if (names_map.find(name) == names_map.end()) {
         // Add actor to actor map if
         // it doesn't exist
       }
       // Add title->actor edge to edge list
       edges.push_back(titles_map[title], names_map[name]);
     }
   }

It will be useful to have adjacency access to both the actors involved
in a movie :math:`Adj[movie]` and the movies and actor is involved in
:math:`Adj[actor]`, so we create adjacency structures for both sets.
Note that since these sets to not overlap, we really have a bipartite
graph.

.. code:: c++

   // Adj[movie]
   auto G = nw::graph::adjacency<0>(edges);
   // Adj[actor]
   auto H = nw::graph::adjacency<1>(edges);

Two actors share a common movie if there is a length two path between
them in this bipartite graph. To make this a direct relationship, we
construct an S overlap edge list which connects actors if this
relationship exists. We will need adjacency lookup into this graph, so
we then compress it to the final costar adjacency structure.

.. code:: c++

   nw::graph::edge_list<nw::graph::undirected, 
     size_t> s_overlap;
   for (size_t i = 0; i < H.size(); ++i) {
     for (auto&& [k] : H[i]) {
       for (auto&& [j] : G[k]) {
         s_overlap.push_back(i, j, k);
       }
     }
   }
   auto costar = nw::graph::adjacency<0, 
     size_t>(s_overlap);

Now we can perform a breadth first search from Kevin Bacon to find the
Bacon number of every actor. This makes use of the bfs_edge_range
adaptor to provide a lightweight view of the BFS traversal.

.. code:: c++

   size_t bacon = names_map["Kevin Bacon"];

   vector<size_t> bacon_number(costar.size());
   vector<size_t> parents(costar.size());
   vector<size_t> together_in(costar.size());

   for (auto&& [u, v, k] : 
     nw::graph::bfs_edge_range(costar, bacon)) {
     bacon_number[v] = bacon_number[u] + 1;
     parents[v]      = u;
     together_in[v]  = k;
   }


After writing a path_to_bacon helper function to traverse the parent tree, we can
query the Bacon number of various actors to find their relationship to Kevin Bacon.


.. code:: c++

  path_to_bacon("Kevin Bacon");
  path_to_bacon("Kyra Sedgwick");
  path_to_bacon("David Suchet");
  path_to_bacon("Julie Kavner");
  path_to_bacon("Samuel L. Jackson");
  path_to_bacon("William Shatner");
  path_to_bacon("Oona O'Neill");


.. code:: 

    Kevin Bacon has a Bacon number of 0
    Kyra Sedgwick has a Bacon number of 1
      Kyra Sedgwick starred with Kevin Bacon in Murder in the First (film)
    David Suchet has a Bacon number of 2
      David Suchet starred with Lori Singer in The Falcon and the Snowman
      Lori Singer starred with Kevin Bacon in Footloose (1984 film)
    Julie Kavner has a Bacon number of 2
      Julie Kavner starred with Dianne Wiest in Radio Days
      Dianne Wiest starred with Kevin Bacon in Footloose (1984 film)
    Samuel L. Jackson has a Bacon number of 2
      Samuel L. Jackson starred with Chris Penn in True Romance
      Chris Penn starred with Kevin Bacon in Footloose (1984 film)
    William Shatner has a Bacon number of 2
      William Shatner starred with Sarah Jessica Parker in Escape from Planet Earth
      Sarah Jessica Parker starred with Kevin Bacon in Footloose (1984 film)
    Oona O'Neill has a Bacon number of 3
      Oona O'Neill starred with Beverly Ross in Broken English (1981 film)
      Beverly Ross starred with Vittorio Gassman in A Wedding
      Vittorio Gassman starred with Kevin Bacon in Sleepers


We can also check the Bacon numbers
of the actors that have played Batman, and find that none of them have a
number greater than two.


.. code:: c++

   path_to_bacon("Adam West");
   path_to_bacon("Michael Keaton");
   path_to_bacon("Val Kilmer");
   path_to_bacon("George Clooney");
   path_to_bacon("Christian Bale");
   path_to_bacon("Ben Affleck");
   path_to_bacon("Robert Pattinson");

.. code:: 

   Adam West has a Bacon number of 2
     Adam West with Frank Welker
     in Aloha, Scooby-Doo!
     Frank Welker with Kevin Bacon
     in Balto (film)
   Michael Keaton has a Bacon number of 1
     Michael Keaton with Kevin Bacon
     in She's Having a Baby
   Val Kilmer has a Bacon number of 2
     Val Kilmer with Brad Renfro
     in 10th & Wolf
     Brad Renfro with Kevin Bacon
     in Sleepers
   George Clooney has a Bacon number of 2
     George Clooney with Minnie Driver
     in South Park: Bigger, Longer & Uncut
     Minnie Driver with Kevin Bacon
     in Sleepers
   Christian Bale has a Bacon number of 2
     Christian Bale with Brad Pitt
     in The Big Short (film)
     Brad Pitt with Kevin Bacon
     in Sleepers
   Ben Affleck has a Bacon number of 2
     Ben Affleck with Minnie Driver
     in Good Will Hunting
     Minnie Driver with Kevin Bacon
     in Sleepers
   Robert Pattinson has a Bacon number of 2
     Robert Pattinson with Jennifer Jason Leigh
     in Good Time (film)
     Jennifer Jason Leigh with Kevin Bacon
     in In the Cut (film)

