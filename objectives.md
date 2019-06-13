# Compare ACO routing with AODV in MANETs
How does the overhead of both algorithms compare?
> ACO routing has more overhead because it includes some proactive elements whereas AODV is purely reactive.

How reliable is ACO routing with respect to link/ node failure when compared to AODV?
> ACO routing is more reliable because it sets up multiple paths and it continues to sample and detect new paths while data is being transmitted, also due to the fact that routing is done stochastically which automatically balances the load across multiple paths. Whereas AODV only sets up one path.

How does the average delay of ACO routing compare to that of AODV?
> Delay will be comparable as the path setup phase uses the same principle in both algorithms (broadcasting a message).

How well does ACO routing scale in MANETs when compared to AODV?
> ACO will scale better 

# Optimise our implementation of ACO routing in MANETs
How does the `s_packetBeta` parameter impact performance?
> Martijn?

How does the acceptance factor impact performance?
> We expect there to be a single sweetspot.

Martijn factor?
> Martijn?

# Discover the impact of the different kinds of agents
What is the impact of the proactive ants?
> Without proactive ants there will be a higher average delay, as there will be less alternative routes.

What is the impact of the route repair ants?
> Without the route repair ant more packtes will be dropped.

What is the combined impact of the route repair and proactive ant?
> Without either of them the protocol should behave more or less like AODV.
