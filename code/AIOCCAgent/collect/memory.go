package collect

import (
	. "../metric"
	"../conf"
	"time"
	"strconv"
)

type MemoryCollector struct {
	*BaseCollectorFiled
	MemoryCollect *MemoryCollect
}

func (memoryCollector *MemoryCollector) CollectorInit(metricQueue *MetricQueue) {
	memoryCollect := &MemoryCollect{}
	memoryCollect.MemoryCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	memoryCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().MemoryCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().MemoryCollectInterval) * time.Second), MetricQueue: metricQueue}
	memoryCollector.MemoryCollect = memoryCollect
}

func (memoryCollector *MemoryCollector) DoCollect() {
	swapMemoryInfo, err1 := memoryCollector.MemoryCollect.GetSwapMemoryInfo()
	memoryInfo, err2 := memoryCollector.MemoryCollect.GetMemoryInfo()
	if err1 == nil && err2 == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		metricData := Metric{
			Dimensions: map[string]string{
				"uuid": vmuuid, "ip": vmip, "namespace": memoryCollector.MetricHandler.NameSpace, "metricName": "system.memory", "deviceName": "", "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
			},

			Values: []map[string]string{
				{"name": "swapMemoryTotal", "value": swapMemoryInfo["swapMemoryTotal"], "unit": "b", },
				{"name": "swapMemoryFree", "value": swapMemoryInfo["swapMemoryFree"], "unit": "b", },
				{"name": "swapMemoryTotal", "value": swapMemoryInfo["swapMemoryTotal"], "unit": "b", },
				{"name": "swapMemorySin", "value": swapMemoryInfo["swapMemorySin"], "unit": "bps", },
				{"name": "swapMemorySout", "value": swapMemoryInfo["swapMemorySout"], "unit": "bps"},
				{"name": "memoryTotal", "value": memoryInfo["memoryTotal"], "unit": "b", },
				{"name": "memoryFree", "value": memoryInfo["memoryFree"], "unit": "b", },
				{"name": "memoryAvailable", "value": memoryInfo["memoryAvailable"], "unit": "b", },
				{"name": "memoryUsed", "value": memoryInfo["memoryUsed"], "unit": "b", },
				{"name": "memoryInactive", "value": memoryInfo["memoryInactive"], "unit": "b", },
				{"name": "memoryActive", "value": memoryInfo["memoryActive"], "unit": "b", },
				{"name": "memoryBuffers", "value": memoryInfo["memoryBuffers"], "unit": "b", },
				{"name": "memoryCached", "value": memoryInfo["memoryCached"], "unit": "b", },
				{"name": "memoryDirty", "value": memoryInfo["memoryDirty"], "unit": "b", },
				{"name": "memoryShared", "value": memoryInfo["memoryShared"], "unit": "b", },
				{"name": "memorySlab", "value": memoryInfo["memorySlab"], "unit": "b", },
				{"name": "memorySwapCached", "value": memoryInfo["memorySwapCached"], "unit": "b", },
				{"name": "memoryWired", "value": memoryInfo["memoryWired"], "unit": "b", },
				{"name": "memoryWriteback", "value": memoryInfo["memoryWriteback"], "unit": "b", },
				{"name": "memoryWritebackTmp", "value": memoryInfo["memoryWritebackTmp"], "unit": "b", },
			}}
		memoryCollector.MetricHandler.AddMetric(&metricData)
		for _, e := range memoryCollector.MetricHandler.PopMetrics() {
			memoryCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (memoryCollector *MemoryCollector) GetCollectorTicker() *time.Ticker {
	return memoryCollector.Ticker
}

func init() {
	RegisterPlugin("memoryCollector", func() BaseCollector {
		return new(MemoryCollector)
	})
}
