## Paper 




## Code/Experimentation

### Installing Cassandara

Set environmental variables
```
cd code
source scripts/setvars.sh
```
Run installation script.
```
$ scripts/install_cassandra.sh
```
If installation completes, tests should succeed

Now, setup DB. If prompted about nodestatus, just press q.
```
scripts/setupdb.sh
```

Now, run using the compiled Cassandra

```
scripts/run.sh
```


### Running YCSB benchmark
```
$ source $CODE/scripts/setvars.sh
$ $CODE/run.sh
```

Sample output of running YCSB benchmark
```
[OVERALL], RunTime(ms), 3577.0
[OVERALL], Throughput(ops/sec), 279.56388034665923
[READ], Operations, 797.0
[READ], AverageLatency(us), 632.5131744040151
....
[READ-MODIFY-WRITE], Operations, 391.0
[READ-MODIFY-WRITE], AverageLatency(us), 1098.7928388746802
.....
[CLEANUP], Operations, 1.0
[CLEANUP], AverageLatency(us), 2239488.0
......
[UPDATE], Operations, 594.0
[UPDATE], AverageLatency(us), 466.68518518518516
...
```
### Running Cassandara across multiple clusters

First set the IP address of different server in scripts/setvars.sh
```
  //Add other servers in the cluster with comma separation
$ export SERVERS="128.104.222.92, 128.104.222.93"
```
Run the cluster setup script. This script will set the cassandara.yaml 
files configuration parameters
```
$ $CODE/scripts/run_cluster_ycsb.sh
```
The script will most likely file if the servers cannot directly connect. </ br>
So, simply add a server's public key to all the other server's ~/.ssh/authorized_keys </ br>

Retry again!

```
$ $CODE/scripts/run_cluster_ycsb.sh
```

If everything goes fine
```
Datacenter: dc1
===============
Status=Up/Down
|/ State=Normal/Leaving/Joining/Moving
--  Address         Load       Tokens       Owns (effective)  Host ID                               Rack
UN  128.104.222.92  20.97 MiB  256          53.6%             8bb46142-9774-4ba8-b6cf-52cce9d619a7  rack1
UN  128.104.222.93  78.86 KiB  256          46.4%             7b5b3781-d72c-41bf-8a9e-053cbc5f5a5d  rack1
```



















