# dlsrest

Distributed Lock System project repository

## Usage

### Using Docker

Requirements: docker and docker-compose

Start the cluster:

```bash
cd docker
docker-compose up
```

Build and run testcases

For faulty follower setting:

```bash
cd docker
docker-compose -f faulty-rep.yml up
```

### From Source

Prerequisites:

```bash
sudo apt-get install libcpprest-dev g++ cmake
```

Build source and testcases:

```bash
mkdir build
cd build
cmake ..
make -j8
```

Run the leader:

```bash
cd build/src
./server <self-address>
```

Run the follower:

```bash
cd build/src
./server <self-address> <leader-address>
```

Run testcases:

```bash
cd build/tests
./test-lock
./test-stress
```
