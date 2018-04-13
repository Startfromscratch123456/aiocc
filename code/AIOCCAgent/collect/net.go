package collect

import (
	. "../metric"
	"../conf"
	"time"
	"strconv"
)

type NetCollector struct {
	*BaseCollectorFiled
	NetCollect *NetCollect
}

func (netCollector *NetCollector) CollectorInit(metricQueue *MetricQueue) {
	netCollect := &NetCollect{}
	netCollect.NetCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	netCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().NetCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().NetCollectInterval) * time.Second), MetricQueue: metricQueue}
	netCollector.NetCollect = netCollect
}

func (netCollector *NetCollector) DoCollect() {
	netInfo, err := netCollector.NetCollect.GetNetIOCounters()
	if err == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		for key, value := range netInfo {
			metricData := Metric{
				Dimensions: map[string]string{
					"uuid": vmuuid, "ip": vmip, "namespace": netCollector.MetricHandler.NameSpace, "metricName": "system.net", "deviceName": key, "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
				},

				Values: []map[string]string{
					//{"name": "deviceName", "value": value["netInterfaceName"], "unit": "", },
					{"name": "netSentTraffic", "value": value["netSentTraffic"], "unit": "bps", },
					{"name": "netRecvTraffic", "value": value["netRecvTraffic"], "unit": "bps", },
					{"name": "netPacketsSent", "value": value["netPacketsSent"], "unit": "countps", },
					{"name": "netPacketsRecv", "value": value["netPacketsRecv"], "unit": "countps", },
					{"name": "netErrin", "value": value["netErrin"], "unit": "countps", },
					{"name": "netErrout", "value": value["netErrout"], "unit": "countps", },
					{"name": "netDropin", "value": value["netDropin"], "unit": "countps", },
					{"name": "netDropout", "value": value["netDropout"], "unit": "countps", },
					{"name": "netFifoin", "value": value["netFifoin"], "unit": "countps", },
					{"name": "netFifoout", "value": value["netFifoout"], "unit": "countps", },
				}}
			netCollector.MetricHandler.AddMetric(&metricData)
		}
		for _, e := range netCollector.MetricHandler.PopMetrics() {
			netCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (netCollector *NetCollector) GetCollectorTicker() *time.Ticker {
	return netCollector.Ticker
}

func init() {
	RegisterPlugin("netCollector", func() BaseCollector {
		return new(NetCollector)
	})
}
