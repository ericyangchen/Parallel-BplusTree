# 2022 Fall Parallel Programming Final Project Report

### 1. Title

Parallelism in Database System: From Tree to Forest

### 2. The participant(s)

- Yang-Jung, Chen eric.cs08@nycu.edu.tw
- Chen-Yu Chang david.cs08@nycu.edu.tw
- Chan-Yu Li frank.cs08@nycu.edu.tw

### 3. Abstract: summarize your contribution in 100 words or less. An informed reader should be able to stop at the abstract and know roughly what you are doing.

In this paper, we propose a method for optimizing the performance of B+ trees, a widely-used data structure in databases and file systems. Our approach involves the use of Pthread workers to parallelize query and insertion tasks. We encountered a problem during parallelized insertions, where inserting into certain subtrees could alter the structure of their parent tree and cause other insertion tasks to fail. To address this issue, we propose a lock strategy that guarantees the integrity of the tree structure while also maximizing performance. Our quantitative analysis demonstrates the effectiveness of our method compared to other approaches.

### 4. Introduction: background on the current state-of-the-art, why your topic is important, and what is the motivation for your work

A parallel B+ tree is a data structure that allows multiple threads or processes to access and modify a B+ tree simultaneously, using parallelism to increase the speed of read and write operations. This can be particularly useful in environments where there is a need to perform a large number of read and write operations on a shared data structure, such as a database index.
The current state of the art for parallel B+ trees involves a number of techniques for increasing the parallelism of read and write operations, including task parallelism, data parallelism, and hybrid approaches that combine both types of parallelism.
One example of parallel B+ tree is the Parallel B+ Tree (PBT), which is a data structure that uses task parallelism to allow multiple threads or processes to access and modify the tree concurrently. PBT uses a divide-and-conquer approach to divide the tree into smaller subtrees, which can be accessed and modified concurrently by different threads or processes.
Most state of the art methods achieve concurrency control on B+ trees using latches, but serialization and contention can affect scalability. As the number of core on processors increases, it is important to develop latch-free techniques for concurrency control. Therefore **Jason et al.** [1] design a method that exploits data and thread-level parallelism on modern many-core architectures to achieve state of the art throughput on concurrent queries.

Overall, the current state of the art in parallel B+ trees involves the use of task parallelism, data parallelism to increase the parallelism of read and write operations. These techniques allow parallel B+ trees to be used in high-performance environments, such as databases and filesystem. 

Due to the fact that database and filesystem are both commonly used application nowadays, the parallelism in B+ tree can have a huge impact on the efficiency of relative applications. Hence we selected parallelism in B+ tree as our topic which is important to many real world application.

### 5. Proposed Solution: detailed description, but not code!

**5-1 Construct B+ tree**

To begin with, we implemented a B+ tree in C++. This tree consists of two types of nodes: intermediate nodes and leaf nodes. The intermediate nodes do not store actual data, but rather contain pointers to their subtree nodes. Each node instance have several components, including keys, pointers to subtree nodes or data, a pointer to its parent node, and a pointer to the next horizontal leaf node (for faster access when querying a range of data).

We also implement several useful functions for the B+ tree, including functions for finding the correct insertion point, inserting into nodes and leaves, and updating nodes as necessary. These functions help us effectively manage the tree and ensure its integrity.

**5-2 Parallelize workload with Pthread**

Parallelization is achieved through the use of Pthreads to spawn worker threads for performing insertions. The number of worker threads ranges from 1 (for serial execution) to 16. Each worker is responsible for a single insertion task, which they retrieve using the C++ build-in atomic function to acquire a task id.

After acquiring the task id, the worker locates the appropriate insertion point in the tree by finding the correct leaf node in which to insert a new key and data pointer. If the leaf node has sufficient space for the insertion, the task is complete. However, this is often not the case due to the self-balancing nature of the B+ tree, which imposes a maximum number of data pointers that each node can store. When a leaf node reaches this maximum capacity, a split must be performed to maintain the balance of the tree. This involves dividing the node in half, selecting the middle key as the new node key, and creating a subtree. This process can introduce a race condition, as multiple worker threads may attempt to update parent nodes and the parents of parent nodes, potentially leading to inconsistency in the tree structure. To address this issue, we implemented two different lock strategies and selected the one that provided the least trade-off.

The first lock strategy operates at the node level, requiring a lock for each node in the entire tree. When an insertion task that requires an update to a parent node with no remaining space is encountered, the worker requests a lock on the parent node and waits until it is acquired. After obtaining the lock, the update is performed. If further updates to the parent node are required, the worker must request a lock on the grandparent node, and so on until all updates have been completed, while holding on to all previously acquired locks. Once all updates are finished, the locks are released. However, this approach has the potential for deadlock, as threads may acquire multiple locks and hold on to locks requested by each other. To avoid this issue, we implemented a second lock strategy that does not suffer from this problem.

The second lock strategy operates at a different level and maintains the same basic concept as the first strategy, but instead of locking individual nodes, it employs locks at the level of each root's subtree. When a worker encounters an insertion task that requires an update to a parent node with no remaining space, it first determines which root's subtree the task will operate on. Since there is a fixed and manageable number of root's subtrees (limited by the number of pointers that a node can store), the number of locks is also fixed. This means that only one thread can perform an insertion on a given root's subtree at a time, limiting the parallelization speedup to the number of root's subtrees. However, the number of root's subtrees is usually orders of magnitude larger than the number of hardware threads, making this limitation trivial in practice. By restricting each root's subtree to be modified by a single thread, this strategy ensures consistency across threads and eliminates the possibility of inconsistency in the tree structure caused by multiple threads. The only potential source of inconsistency in the tree structure is the root itself, as each thread may need to update the root's key. To address this issue, we implemented a read-write lock on the root node. When an insertion requires an update to the root node, the root lock must be acquired. Once obtained, no other thread can perform an insertion until it is released, ensuring the consistency of the root structure across threads.

Given the defects of the first lock method described above, we chose to implement the second method as it is easier to implement and requires significantly fewer locks. While it may offer slightly less parallelization compared to the first method, the reduced locking overhead makes it a reasonable trade-off in terms of maximum parallelization. Overall, we believe that the second lock strategy represents a good balance between efficiency and simplicity.

**5-3  Lock implementation**

To ensure consistency in the parallelized B+tree, it is necessary to implement read locks on the root's structure when accessing it and write locks when updating it. This will prevent conflicting accesses and updates, ensuring that the data remains accurate and consistent. When a thread tries to update the root's structure, it is necessary to use a write lock on the entire root. This will prevent any other threads from accessing or modifying the root's structure while the update is being performed, ensuring that the data remains consistent and accurate. After the update is complete, the write lock can be released to allow other threads to access the root's structure again.

In the Pthread library, there are two types of read-write locks: blocking and non-blocking. However, this implementation does not guarantee that write operations will be performed first, potentially leading to write starvation. If write starvation occurs, subsequent insertions may also require write operations due to a lack of space in the tree. To address this issue, we implemented a write-preferring read-write lock based on [2] using multiple mutexes, which ensures the priority of write operations.

This is an important consideration when implementing a parallel B+ tree, as write starvation can significantly impact the performance of the tree. By prioritizing write operations, we can ensure that the tree remains balanced and maintain good performance. Our write-preferring read-write lock can be a useful tool for achieving this goal in parallel B+ tree implementation.

### 6. Experimental Methodology: tests, input sets, environment, etc.

To evaluate the performance, we will measure the average execution time. We will conduct each experiment six times and calculate the average execution time based on the top four minimum execution times. Additionally, we will verify the correctness of our B+ tree implementation by comparing the query results with the serial version.

The input set for this experiment consists of 5,000,000 rows of data for insertion, 1,000,000 key queries, and 500,000 range queries. The insertion task involves inserting a specified key-value pair into the B+ tree. The key query task involves retrieving the value of a specified key. The range query task involves retrieving the maximum value within a specified key range. We conducted these three tasks separately and evaluated their performance.

First, we need to determine the order of the B+ trees. The order can significantly affect the performance due to the resulting differences in the depth of the B+ trees. After identifying an optimal order that yields the least execution time, we will use this order for the remainder of the experiments.
Next, we will compare the performance of the serial and parallel versions of the B+ trees. We have implemented three different parallel versions of the B+ trees: one using the blocking read-write lock with the Pthread library, another using the non-blocking read-write lock with the Pthread library, and the third using our own implementation of the blocking read-write lock.

In this study, the testing experiments were performed on a desktop computer equipped with an AMD Ryzen 5 5600x CPU featuring 6 cores and 12 threads, as well as 32 GB of RAM. The system was running the Windows 10 operating system. The programs were built using MinGW-w64 [3] and GCC 12.2.0, with the POSIX Pthread library for threading support.

### 7. Experimental Results: Quantitative data and analysis!

**Order of B+ trees**

| Order | 32 | 64 | 128 | 256 | 512 |
| --- | --- | --- | --- | --- | --- |
| Execution Time (ms) | 1955.16 | 1910.66 | 1965.06 | 2412.94 | 3054.06 |

Table 1 presents the results of the experiments with different orders for the B+ tree in the insertion task. As can be seen, setting the order to 64 yields the best performance, with an average execution time of 1910.66 milliseconds. This may be due to the fact that the B+ tree has a smaller depth at this order, which can reduce the time required for insertion operations.

It is worth noting that the execution time increases as the order of the B+ tree increases. This may be because a larger order results in a deeper tree, which can increase the time required for insertion due to the need for more node traversals. Additionally, a larger order may also lead to more internal nodes, which can increase the overhead for maintaining the tree structure. Overall, these results suggest that it is important to carefully consider the trade-offs between the order of the B+ tree and its performance in insertion tasks.

**Insertion Task**

| Insertion | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- |
| Serial | 1910.66 |  |  |  |  |  |
| Pthread Blocking | 2014.68 (0.94x) | 1542.87(1.23x) | 1589.70(1.20x) | 1984.64(0.96x) | 2451.22(0.77x) | 2937.49(0.65x) |
| Pthread Non-blocking | 2075.55(0.92x) | 1581.38(1.20x) | 1753.71(1.08x) | 2126.74(0.89x) | 2495.33(0.76x) | 2872.54(0.67x) |
| Ours (Blocking) | 1987.43 (0.96x) | 1561.17(1.22x) | 1810.89(1.05x) | 1993.18(0.95x) | 2047.99(0.93x) | 2073.13(0.92x) |

We evaluated the effect of different read-write lock implementations on the performance of B+ tree insertion and present the results in Table 2. We observed that the best performance is achieved when using 2 threads. One possible explanation for the increased execution time with 3 or more threads is the overhead of the read-write lock for the root node. As the number of threads increases, the overhead becomes larger, significantly impacting the execution time. Therefore, we achieved the maximum speedup of 1.23x with 2 threads.

**Query Task**

| Key Query | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- |
| Serial | 385.94 |  |  |  |  |  |
| Parallel | 367.05(1.05x) | 205.83(1.87x) | 150.93(2.56x) | 108.30(3.56x) | 89.74(4.30x) | 77.28(4.99x) |

| Range Query | 1 | 2 | 3 | 4 | 5 | 6 |
| --- | --- | --- | --- | --- | --- | --- |
| Serial | 652.60 |  |  |  |  |  |
| Parallel | 653.84(1.00x) | 350.72(1.86x) | 256.88(2.54x) | 197.62(3.30x) | 169.44(3.85x) | 152.87(4.27x) |

By comparing our method with the serial version, we found that the performance of key queries and range queries is as expected. Although there is some overhead when allocating tasks to threads, these results demonstrate that the advantage of our parallel B+ tree implementation increases with a larger number of threads. Despite the overhead, the overall performance is still improved when using more threads.

### 8. Related work: Relate your work to research by others. Any time you mention some other work, compare or contrast it to your own.

**PALM: Parallel Architecture-Friendly Latch-Free Modifications to B+ Trees on Many-Core Processors [1]**

The paper presents a technique called PALM for performing multiple concurrent queries on in-memory B+ trees, which are commonly used in databases. PALM is based on the Bulk Synchronous Parallel model, which ensures freedom from deadlocks and race conditions, and groups and processes queries in atomic batches. It is designed to exploit data- and thread-level parallelism on modern many-core architectures, and is able to perform 40 million updates per second on trees with 128 million keys and 128 million updates per second on trees with 512 thousand keys on current CPU architectures. It also has low response times of less than 350 microseconds, even for large trees. The paper also demonstrates that PALM has a speedup of 1.5-2.1 times on the Intel Many Integrated Core architecture compared to a pair of Intel Xeon processors. To sum, PALM is a scalable and efficient technique for concurrency control on B+ trees.

Inspired by this work, we decide to use multi-thread on multi-core CPU to achieve thread-level parallelism. Although the speedup of our method is far from that of PALM, we implement extra concurrent operations such as concurrent insertion and concurrent queries.

**Efficient *Locking for Concurrent Operations* on B-Trees [4]**

The paper presents a modified version of B+ tree, B-link tree, which is shown in different works [5] to be superior to the original B+ tree. B-link tree adds a link pointer which points to the next node at the same level of the tree as the current node and an extra high key within every node. Link pointer can help avoiding global lock while adjusting the tree structure. With these extra properties, only a constant number of locks need be used by any process at any time. 

In contrast to this work, we doesn’t modify the tree data structure. Therefore the speedup may not be as significant, but the intuition is much easier to grasp.

### 9. Conclusions: Highlight the important points of your analysis and contribution. Also give prospects for future research on this or related topics.

In this paper, we evaluated the performance of a parallel B+ tree implementation for various tasks, including insertions, key queries, and range queries. We first analyzed the impact of B+ tree orders on overall performance, then compared the performance of our approach with that of a serial version using different read-write lock implementations.
The results showed that the best insertion performance was achieved using two threads and setting the order of the B+ tree to 64, possibly due to the overhead of the read-write lock. On the other hand, the performance of range and key queries improved as the number of threads increased, demonstrating the effectiveness of our parallel implementation despite the overhead of task allocation.

Overall, our results demonstrate the potential of parallel B+ tree implementation to improve the performance of various tasks and highlight the importance of carefully considering the trade-offs between different design choices. Future research could focus on identifying additional optimization strategies for parallel B+ tree implementation to further improve its performance and on designing a better lock strategy.

### 10. References

[1] J. Sewall, J. Chhugani and C. Kim. , 2011. PALM: Parallel Architecture-Friendly Latch-Free Modifications to B+ Trees on Many-Core Processors DOI: [http://www.vldb.org/pvldb/vol4/p795-sewall.pdf](http://www.vldb.org/pvldb/vol4/p795-sewall.pdf)

[3] niXman. 2022. Releases · niXman/mingw-builds-binaries DOI: [https://github.com/niXman/mingw-builds-binaries/releases](https://github.com/niXman/mingw-builds-binaries/releases) 

[4] PL LEHMAN. 1981. Efficient Locking for Concurrent Operations on B-Trees DOI: [https://www.csd.uoc.gr/~hy460/pdf/p650-lehman.pdf](https://www.csd.uoc.gr/~hy460/pdf/p650-lehman.pdf)

[5] V. Srinivasan and Michael J. Carey, 1991. Performance of B + Tree Concurrency Control Algorithms DOI:[https://www.vldb.org/journal/VLDBJ2/P361.pdf](https://www.vldb.org/journal/VLDBJ2/P361.pdf)

[6]     Patricia S. Abril and Robert Plant, 2007. The patent holder's dilemma: Buy, sell, or troll? *Commun. ACM* 50, 1 (Jan, 2007), 36-44. DOI: https://doi.org/10.1145/1188913.1188915.

[7]     Sten Andler. 1979. Predicate path expressions. In *Proceedings of the 6th. ACM SIGACT-SIGPLAN Symposium on Principles of Programming Languages (POPL '79)*. ACM Press, New York, NY, 226-236. DOI:https://doi.org/10.1145/567752.567774

[8]   Ian Editor (Ed.). 2007. *The title of book one* (1st. ed.). The name of the series one, Vol. 9. University of Chicago Press, Chicago. DOI:https://doi.org/10.1007/3-540-09237-4.

[9]     David Kosiur. 2001. *Understanding Policy-Based Networking* (2nd. ed.). Wiley, New York, NY..
