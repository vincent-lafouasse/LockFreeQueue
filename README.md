# LockFreeQueue

A review on wait-free data structures

## size configuration

consumers must provide CLF_QUEUE_SIZE one of two ways:
1. command line:          `cmake -DCLF_QUEUE_SIZE=8192 [...]`
2. downstream CML.txt:    `set(CLF_QUEUE_SIZE 8192 CACHE STRING "" FORCE)`
    - (FORCE required to override any cached value)

the absence of a default value is intentional. you MUST provide a size

## References

- \[Swartz2008\] A. Swartz, "Guerilla Open Access Manifesto," 2008. [Archive](https://archive.org/details/GuerillaOpenAccessManifesto).

- \[Lamport1983\] L. Lamport, "Specifying Concurrent Program Modules," *ACM Trans. Program. Lang. Syst.*, vol. 5, no. 2, pp. 190–222, Apr. 1983. DOI: [10.1145/69624.357207](https://doi.org/10.1145/69624.357207). [Archive](https://lamport.azurewebsites.net/pubs/spec.pdf).

- \[MichaelScott1996\] M. M. Michael and M. L. Scott, "Simple, fast, and practical non-blocking and blocking concurrent queue algorithms," in *Proc. 15th Annu. ACM Symp. Princ. Distrib. Comput. (PODC '96)*, pp. 267–275, 1996. DOI: [10.1145/248052.248106](https://doi.org/10.1145/248052.248106). [Archive](https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf).

- \[LadanMozes2004\] E. Ladan-Mozes and N. Shavit, "An Optimistic Approach to Lock-Free FIFO Queues," in *Proc. 18th Int. Symp. Distrib. Comput. (DISC '04)*, pp. 117–131, 2004. DOI: [10.1007/978-3-540-30186-8_9](https://doi.org/10.1007/978-3-540-30186-8_9). [Archive](https://people.csail.mit.edu/shanir/publications/FIFO_Queues.pdf).

- \[Giacomoni2008\] J. Giacomoni, T. Moseley, and M. Vachharajani, "FastForward for efficient pipeline parallelism: a cache-optimized concurrent lock-free queue," in *Proc. 13th ACM SIGPLAN Symp. Princ. Pract. Parallel Program. (PPoPP '08)*, pp. 43–52, 2008. DOI: [10.1145/1345206.1345215](https://doi.org/10.1145/1345206.1345215). [Archive](https://dl.acm.org/doi/epdf/10.1145/1345206.1345215).

- \[Aldinucci2012\] M. Aldinucci, M. Danelutto, P. Kilpatrick, M. Meneghin, and M. Torquati, "An Efficient Unbounded Lock-Free Queue for Multi-core Systems," in *Proc. 18th Int. Conf. Parallel Process. (Euro-Par '12)*, pp. 662–673, 2012. DOI: [10.1007/978-3-642-32820-6_65](https://doi.org/10.1007/978-3-642-32820-6_65). [Archive](https://link.springer.com/content/pdf/10.1007/978-3-642-32820-6_65.pdf).

- \[Le2013\] N. M. Lê, A. Guatto, A. Cohen, and A. Pop, "Correct and Efficient Bounded FIFO Queues," in *Proc. 25th Int. Symp. Comp. Archit. High Perform. Comput. (SBAC-PAD '13)*, pp. 144–151, 2013. DOI: [10.1109/SBAC-PAD.2013.8](https://doi.org/10.1109/SBAC-PAD.2013.8). [Archive](https://www.irif.fr/~guatto/publications/sbac13.pdf).
