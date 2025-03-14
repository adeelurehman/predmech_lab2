spec=(429.mcf 450.soplex 471.omnetpp 473.astar 482.sphinx3 483.xalancbmk)
gap=(bfs cc)
# benchmarks=(473.astar)
time_value=$(date +%H%M)
mkdir "$time_value"

for benchmark in "${spec[@]}"; do
    mkdir "$time_value/$benchmark"
    ./gem5-configs-395t/scripts/run-spec06-se-periodic.py "$benchmark" \
        --gem5-binary build/X86/gem5.opt \
        --outdir "$time_value/$benchmark" \
        --spec06-dir spec06 \
        --ff-interval 10000 \
        --warmup-interval 25 \
        --roi-interval 50 \
        --init-ff-interval 10000 \
        --num-rois 3 \
        --redirect &
done

mkdir "$time_value/bfs"
./build/X86/gem5-opt \
    -- gem5-configs-395t/se_custom_binary_periodic.py \
    --input-bin "spec06/bfs" \
    --input-args "-r 1 -f /scratch/cluster/speedway/cs395t/hw2/part2/gap/g22.el" \
    --init-ff-interval 10000 \
    --ff-interval 10000 \
    --warmup-interval 25 \
    --roi-interval 50 \
    --num-rois 3 \
    > "$time_value/bfs.stdout" &

mkdir "$time_value/cc"
./build/X86/gem5-opt \
    -- gem5-configs-395t/se_custom_binary_periodic.py \
    --input-bin "spec06/cc" \
    --input-args "-r 1 -f /scratch/cluster/speedway/cs395t/hw2/part2/gap/g22.el" \
    --init-ff-interval 10000 \
    --ff-interval 10000 \
    --warmup-interval 25 \
    --roi-interval 50 \
    --num-rois 3 \
    > "$time_value/cc.stdout" &
