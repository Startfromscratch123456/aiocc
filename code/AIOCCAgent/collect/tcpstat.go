package collect

import (
	"time"
	"../conf"
	. "../metric"
	"strconv"
)

type TcpCollector struct {
	*BaseCollectorFiled
	TcpCollect *TcpCollect
}

func (tcpCollector *TcpCollector) CollectorInit(metricQueue *MetricQueue) {
	tcpCollect := &TcpCollect{}
	tcpCollect.TcpCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	tcpCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().TcpCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().TcpCollectInterval) * time.Second), MetricQueue: metricQueue}
	tcpCollector.TcpCollect = tcpCollect
}

func (tcpCollector *TcpCollector) DoCollect() {
	tcpCurrEstab, err1 := tcpCollector.TcpCollect.GetTcpCurrEstab()
	tcpConnPerSec, err2 := tcpCollector.TcpCollect.GetTcpConnInc()
	tcpTimeAwait, err3 := tcpCollector.TcpCollect.GetTimeAwait()
	if err1 == nil && err2 == nil && err3 == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		metricData := Metric{
			Dimensions: map[string]string{
				"uuid": vmuuid, "ip": vmip, "namespace": tcpCollector.MetricHandler.NameSpace, "metricName": "system.tcpstat", "deviceName": "", "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
			},

			Values: []map[string]string{
				{"name": "tcpCurrEstab", "value": strconv.FormatInt(tcpCurrEstab, 10), "unit": "count", },
				{"name": "tcpConnPerSec", "value": strconv.FormatFloat(tcpConnPerSec, 'f', -1, 64), "unit": "count", },
				{"name": "tcpTimeAwait", "value": strconv.FormatInt(tcpTimeAwait, 10), "unit": "count", },

			}}
		tcpCollector.MetricHandler.AddMetric(&metricData)
		for _, e := range tcpCollector.MetricHandler.PopMetrics() {
			tcpCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (tcpCollector *TcpCollector) GetCollectorTicker() *time.Ticker {
	return tcpCollector.Ticker
}

func init() {
	RegisterPlugin("tcpCollector", func() BaseCollector {
		return new(TcpCollector)
	})
}
