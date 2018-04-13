package collect

import (
	. "../metric"
	"../conf"
	"time"
	"strconv"
)

type SystemCollector struct {
	*BaseCollectorFiled
	SystemCollect *SystemCollect
}

func (systemCollector *SystemCollector) CollectorInit(metricQueue *MetricQueue) {
	systemCollect := &SystemCollect{}
	systemCollect.SystemCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	systemCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().SystemCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().SystemCollectInterval) * time.Second), MetricQueue: metricQueue}
	systemCollector.SystemCollect = systemCollect
}

func (systemCollector *SystemCollector) DoCollect() {
	runTime, rebooted, err := systemCollector.SystemCollect.GetRunTime()
	if err == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		metricData := Metric{
			Dimensions: map[string]string{
				"uuid": vmuuid, "ip": vmip, "namespace": systemCollector.MetricHandler.NameSpace, "metricName": "system.system", "deviceName": "", "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
			},

			Values: []map[string]string{
				{"name": "systemRunTime", "value": strconv.FormatFloat(runTime, 'f', -1, 64), "unit": "s", },
				{"name": "systemRebooted", "value": strconv.FormatBool(rebooted), "unit": "", },
			}}
		systemCollector.MetricHandler.AddMetric(&metricData)
		for _, e := range systemCollector.MetricHandler.PopMetrics() {
			systemCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (systemCollector *SystemCollector) GetCollectorTicker() *time.Ticker {
	return systemCollector.Ticker
}

func init() {
	RegisterPlugin("systemCollector", func() BaseCollector {
		return new(SystemCollector)
	})
}
