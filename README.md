mean_keaper
===========

Helper data structure for image segmentation algorithm. Distributes resources among consumers. Each resource can be owned only by one consumer. A consumer gets resources if its priority (that depends on its resources) is higher than of its competitors. Supports fast approximate removal and addition of new consumers.
