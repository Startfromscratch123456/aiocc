package metric

type Metric struct {
	Dimensions map[string]string `json:"Dimensions"`
	Values     []map[string]string `json:"Values"`
}
type MetricHandler struct {
	Metrics   []*Metric
	NameSpace string
}

func (metricHandler *MetricHandler) MetricHandlerInit(ns string) {
	metricHandler.Metrics = make([]*Metric, 10)
	metricHandler.NameSpace = ns
}

func (metricHandler *MetricHandler) AddMetric(metric *Metric) {
	metricHandler.Metrics = append(metricHandler.Metrics, metric)
}

func (metricHandler *MetricHandler) GetMetrics() ([]*Metric) {
	return metricHandler.Metrics
}

func (metricHandler *MetricHandler) ClearMetrics() {
	metricHandler.Metrics = make([]*Metric, 10)
}

func (metricHandler *MetricHandler) PopMetrics() ([]*Metric) {
	ret := metricHandler.GetMetrics()
	metricHandler.ClearMetrics()
	return ret
}
