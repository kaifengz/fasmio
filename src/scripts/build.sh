#!/bin/bash

pub_dir="../output"
pub_name="fasmio"
pub_files="main/fasmio"
pub_files_plugin="service/registry/libregistry.so"
build_dir="../build"


function build()
{
    mkdir -p $pub_dir

    rm -rf $build_dir/
    mkdir -p $build_dir/$pub_name/
    mkdir -p $build_dir/$pub_name/plugin/

    cp $pub_files            $build_dir/$pub_name/
    cp $pub_files_plugin     $build_dir/$pub_name/plugin/

    pushd $build_dir/$pub_name/   > /dev/null
    tar cfz ../$pub_name.tgz  *
    popd   > /dev/null

    mv $build_dir/$pub_name.tgz  $pub_dir/
    cat scripts/install.sh $pub_dir/$pub_name.tgz > $pub_dir/install-$pub_name.sh
    chmod +x $pub_dir/install-$pub_name.sh
}

build

