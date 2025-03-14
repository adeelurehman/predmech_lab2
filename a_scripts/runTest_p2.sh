spec=(429.mcf 450.soplex 471.omnetpp 473.astar 482.sphinx3 483.xalancbmk)
# spec=(473.astar)
time_value=$(date +%H%M)
root=${PWD}
mkdir "$root/$time_value"
cd gem5-configs-395t

for benchmark in "${spec[@]}"; do
    mkdir "$root/$time_value/$benchmark"
    ./scripts/run-spec06-se-periodic.py "$benchmark" \
        --gem5-binary ../build/X86/gem5.opt \
        --outdir "$root/$time_value/$benchmark" \
        --spec06-dir "$root/spec06" \
        --ff-interval 10000 \
        --warmup-interval 25 \
        --roi-interval 50 \
        --init-ff-interval 10000 \
        --num-rois 3 \
        --redirect &
done

 mkdir "$root/$time_value/bfs"
 $root/build/X86/gem5.opt \
     -- "$root/gem5-configs-395t/se_custom_binary_periodic.py" \
     --input-bin "$root/spec06/bfs" \
     --input-args "-r 1 -f /scratch/cluster/speedway/cs395t/hw2/part2/gap/g22.el" \
     --init-ff-interval 10000 \
     --ff-interval 10000 \
     --warmup-interval 25 \
     --roi-interval 50 \
     --num-rois 3 \
     > "$root/$time_value/bfs.stdout" 2>&1 &

 mkdir "$root/$time_value/cc"
 $root/build/X86/gem5.opt \
     -- "$root/gem5-configs-395t/se_custom_binary_periodic.py" \
     --input-bin "$root/spec06/cc" \
     --input-args "-r 1 -f /scratch/cluster/speedway/cs395t/hw2/part2/gap/g22.el" \
     --init-ff-interval 10000 \
     --ff-interval 10000 \
     --warmup-interval 25 \
     --roi-interval 50 \
     --num-rois 3 \
     > "$root/$time_value/cc.stdout" 2>&1 &

cd ..
