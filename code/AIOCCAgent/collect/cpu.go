package collect

import (
	. "../metric"
	"../conf"
	"time"
	"strconv"
	"strings"
)

type CpuCollector struct {
	*BaseCollectorFiled
	CpuCollect *CpuCollect
}

func (cpuCollector *CpuCollector) CollectorInit(metricQueue *MetricQueue) {
	cpuCollect := &CpuCollect{}
	cpuCollect.CpuCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	cpuCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().CpuCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().CpuCollectInterval) * time.Second), MetricQueue: metricQueue}
	cpuCollector.CpuCollect = cpuCollect
}

func (cpuCollector *CpuCollector) DoCollect() {
	if cpusInfo, err := cpuCollector.CpuCollect.GetCpuInfo(); err == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		for key, value := range cpusInfo {
			metricData := Metric{
				Dimensions: map[string]string{
					"uuid": vmuuid, "ip": vmip, "namespace": cpuCollector.MetricHandler.NameSpace, "metricName": "system.cpu", "deviceName": key, "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
				},

				Values: []map[string]string{
					//{"name": "deviceName", "value": key, "unit": "", },
					{"name": "cpuUser", "value": value["cpuUser"], "unit": "%", },
					{"name": "cpuIdle", "value": value["cpuIdle"], "unit": "%", },
					{"name": "cpuSystem", "value": value["cpuSystem"], "unit": "%", },
					{"name": "cpuNice", "value": value["cpuNice"], "unit": "%", },
					{"name": "cpuIowait", "value": value["cpuIowait"], "unit": "%", },
					{"name": "cpuIrq", "value": value["cpuIrq"], "unit": "%", },
					{"name": "cpuSoftirq", "value": value["cpuSoftirq"], "unit": "%", },
					{"name": "cpuSteal", "value": value["cpuSteal"], "unit": "%", },
					{"name": "cpuGuest", "value": value["cpuGuest"], "unit": "%", },
					{"name": "cpuGuestNice", "value": value["cpuGuestNice"], "unit": "%", },
					{"name": "cpuStolen", "value": value["cpuStolen"], "unit": "%", },
					{"name": "cpuPercent", "value": value["cpuPercent"], "unit": "%", },
				}}

			if strings.Contains(key, "cpu-total") {
				metricData.Values = append(metricData.Values, map[string]string{"name": "cpuLoadAverage1", "value": value["cpuLoadAverage1"], "unit": "%", })
				metricData.Values = append(metricData.Values, map[string]string{"name": "cpuLoadAverage5", "value": value["cpuLoadAverage5"], "unit": "%", })
				metricData.Values = append(metricData.Values, map[string]string{"name": "cpuLoadAverage15", "value": value["cpuLoadAverage15"], "unit": "%", })
				metricData.Values = append(metricData.Values, map[string]string{"name": "cpuRunningProcesses", "value": value["cpuRunningProcesses"], "unit": "count", })
				metricData.Values = append(metricData.Values, map[string]string{"name": "cpuTotalProcesses", "value": value["cpuTotalProcesses"], "unit": "count", }, )
				metricData.Values = append(metricData.Values, map[string]string{"name": "cpuLastProcessId", "value": value["cpuLastProcessId"], "unit": "", })
			}
			cpuCollector.MetricHandler.AddMetric(&metricData)
		}

		for _, e := range cpuCollector.MetricHandler.PopMetrics() {
			cpuCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (cpuCollector *CpuCollector) GetCollectorTicker() *time.Ticker {
	return cpuCollector.Ticker
}

func init() {
	RegisterPlugin("cpuCollector", func() BaseCollector {
		return new(CpuCollector)
	})
}
