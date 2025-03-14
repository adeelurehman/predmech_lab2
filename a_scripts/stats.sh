# call from the git repo root
benches=(429.mcf 450.soplex 471.omnetpp 473.astar 482.sphinx3 483.xalancbmk bfs cc)
cd $1

for benchmark in "${benches[@]}"; do
    echo $benchmark
    cat $benchmark/stats.txt | grep -E "switch.*ipc" | grep -v commitStats
    cat $benchmark/stats.txt | grep -E "simInsts"
    cat $benchmark/stats.txt | grep -E "llcache.*overallHits.*total"
    cat $benchmark/stats.txt | grep -E "llcache.*overallMisses.*total"
    cat $benchmark/stats.txt | grep -E "llcache.*overallAccesses.*total"
    echo ""
done
