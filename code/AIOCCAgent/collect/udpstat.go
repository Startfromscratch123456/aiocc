package collect

import (
	"time"
	"../conf"
	. "../metric"
	"strconv"
)

type UdpCollector struct {
	*BaseCollectorFiled
	UdpCollect *UdpCollect
}

func (udpCollector *UdpCollector) CollectorInit(metricQueue *MetricQueue) {
	udpCollect := &UdpCollect{}
	udpCollect.UdpCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	udpCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().UdpCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().UdpCollectInterval) * time.Second), MetricQueue: metricQueue}
	udpCollector.UdpCollect = udpCollect
}

func (udpCollector *UdpCollector) DoCollect() {
	udpInfo, err := udpCollector.UdpCollect.GetUdpInfo()
	if err == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		metricData := Metric{
			Dimensions: map[string]string{
				"uuid": vmuuid, "ip": vmip, "namespace": udpCollector.MetricHandler.NameSpace, "metricName": "system.udpstat", "deviceName": "", "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
			},

			Values: []map[string]string{
				{"name": "udpInDatagrams", "value": udpInfo["udpInDatagrams"], "unit": "countps", },
				{"name": "udpOutDatagrams", "value": udpInfo["udpOutDatagrams"], "unit": "countps", },
				{"name": "udpInErrors", "value": udpInfo["udpInErrors"], "unit": "countps", },
				{"name": "udpRcvbufErrors", "value": udpInfo["udpRcvbufErrors"], "unit": "countps", },
				{"name": "udpSndbufErrors", "value": udpInfo["udpSndbufErrors"], "unit": "countps", },
				{"name": "udpInCsumErrors", "value": udpInfo["udpInCsumErrors"], "unit": "countps", },

			}}
		udpCollector.MetricHandler.AddMetric(&metricData)
		for _, e := range udpCollector.MetricHandler.PopMetrics() {
			udpCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (udpCollector *UdpCollector) GetCollectorTicker() *time.Ticker {
	return udpCollector.Ticker
}

func init() {
	RegisterPlugin("udpCollector", func() BaseCollector {
		return new(UdpCollector)
	})
}
