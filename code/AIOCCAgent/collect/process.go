package collect

import (
	. "../metric"
	"../conf"
	"time"
	"strconv"
)

type ProcessCollector struct {
	*BaseCollectorFiled
	ProcessCollect *ProcessCollect
}

func (processCollector *ProcessCollector) CollectorInit(metricQueue *MetricQueue) {
	processCollect := &ProcessCollect{}
	processCollect.ProcessCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	processCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().ProcessCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().ProcessCollectInterval) * time.Second), MetricQueue: metricQueue}
	processCollector.ProcessCollect = processCollect
}

func (processCollector *ProcessCollector) DoCollect() {
	processList := conf.GetConfigTool().ProcessList
	vmuuid := GetVMUUID()
	vmip := GetVMIP()
	for _, processName := range processList {
		if processInfo, err := processCollector.ProcessCollect.GetProcessInfo(processName); err == nil {
			if err == nil && len(processInfo) > 1 {
				metricData := Metric{
					Dimensions: map[string]string{
						"uuid": vmuuid, "ip": vmip, "namespace": processCollector.MetricHandler.NameSpace, "metricName": "system.process", "deviceName": "", "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
					},
					Values: []map[string]string{
						{"name": "processName", "value": processName, "unit": "", },
						{"name": "processStatus", "value": processInfo["status"], "unit": "", },
						{"name": "processCreateTime", "value": processInfo["createTime"], "unit": "s", },
						//{"name": "processIsRunning", "value": processInfo["isRunning"], "unit": "", },
						{"name": "processPercent", "value": processInfo["percent"], "unit": "%", },
						{"name": "processSwap", "value": processInfo["Swap"], "unit": "byte", },
						{"name": "processVMS", "value": processInfo["VMS"], "unit": "byte", },
						{"name": "processRSS", "value": processInfo["RSS"], "unit": "byte", },
					},
				}
				processCollector.MetricHandler.AddMetric(&metricData)
			}
		}
	}
	for _, e := range processCollector.MetricHandler.PopMetrics() {
		processCollector.MetricQueue.EnQueue(e)
	}

}

func (processCollector *ProcessCollector) GetCollectorTicker() *time.Ticker {
	return processCollector.Ticker
}

func init() {
	RegisterPlugin("processCollector", func() BaseCollector {
		return new(ProcessCollector)
	})
}
