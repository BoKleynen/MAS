# MAS-Project
We used [ns-3](https://www.nsnam.org/) as our simulation environment.
## Prerequisites
- Docker
- make

## Running the simulation
execute
```bash
make run
```

This will download a docker image containing a precompiled version of ns-3 and then compile and execute experiments.

Warning: it takes a very long time to execute all experiments.

Note: On linux you might need to run the above command as root because docker requires root priviliges

