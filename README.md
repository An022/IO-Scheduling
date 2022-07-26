# IO-Scheduling
A programming lab with professor Hubertus Franke. 

We simulate and put into practice I/O operation scheduling and optimization in this lab. Applications send the IO subsystem their IO requests, which are kept in an IO-queue until the disk device is prepared to handle another request. A request is then chosen from the IO-queue and sent to the disk device by the IO-scheduler. In operating systems, this option is sometimes referred to as the strategy() routine and is seen in the figure below. After finishing, a different request might be pulled from the IO-queue and sent to the disk. The scheduling rules will permit some system optimization in order to cut down on disk head movement or overall wait times. Only the left side of the figure (I/O Scheduler) is provided for this lab.   

The implemented schedulers are FIFO I SSTF (j), LOOK (s), CLOOK (c), and FLOOK (f). The letters in brackets indicate which parameter has to be specified in the -s program flag.
