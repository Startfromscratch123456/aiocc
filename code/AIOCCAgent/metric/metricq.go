package metric

type MetricQueue struct {
	MetricChan chan *Metric
}

func (metricQueue *MetricQueue) MetricQueueInit() {
	metricQueue.MetricChan = make(chan *Metric, 500)
}

func (metricQueue *MetricQueue) EnQueue(metric *Metric) {
	metricQueue.MetricChan <- metric
}
