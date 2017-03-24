# TODO

* after the 10s wait time after an obstacle is met, the motors turn on again
* member intialization list class
  * done :)
* co je hvezdicka motors -- pointer
* BUG: slow then stop - after 10s starts up again only to be stopped the next instance
  + solution: make global static variable speedPercentage and call runMotors with the var (at the end of the loop)
    - set speedPercentage in switch
  * ignore (only restarts once it gets 1 result with distance over the stop limit)
* merge serial and my serial in log
* add documentation to motor class (add void?)
