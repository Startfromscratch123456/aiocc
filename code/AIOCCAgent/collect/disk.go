package collect

import (
	. "../metric"
	"../conf"
	"time"
	"strconv"
)

type DiskCollector struct {
	*BaseCollectorFiled
	DiskCollect *DiskCollect
}

func (diskCollector *DiskCollector) CollectorInit(metricQueue *MetricQueue) {
	diskCollect := &DiskCollect{}
	diskCollect.DiskCollectInit()
	metricHandler := &MetricHandler{}
	metricHandler.MetricHandlerInit("dcs/cvm") //didi cloud service : cloud virtual machine
	diskCollector.BaseCollectorFiled = &BaseCollectorFiled{CollectInterval: conf.GetConfigTool().DiskCollectInterval, MetricHandler: metricHandler, Ticker: time.NewTicker(time.Duration(conf.GetConfigTool().DiskCollectInterval) * time.Second), MetricQueue: metricQueue}
	diskCollector.DiskCollect = diskCollect
}

func (diskCollector *DiskCollector) DoCollect() {
	allDiskPartitionInfo, err := diskCollector.DiskCollect.GetPartitionInfo()
	if err == nil {
		vmuuid := GetVMUUID()
		vmip := GetVMIP()
		for diskName, diskPartitionInfo := range allDiskPartitionInfo {
			metricData := Metric{
				Dimensions: map[string]string{
					"uuid": vmuuid, "ip": vmip, "namespace": diskCollector.MetricHandler.NameSpace, "metricName": "system.disk", "deviceName": diskName, "timeStamp": strconv.FormatInt(time.Now().Unix(), 10), "interval": strconv.FormatInt(conf.GetConfigTool().UdpCollectInterval, 10),
				},

				Values: []map[string]string{
					//{"name": "deviceName", "value": diskName, "unit": "", },
					{"name": "diskType", "value": diskPartitionInfo["diskType"], "unit": "", },
					{"name": "diskTotal", "value": diskPartitionInfo["diskTotal"], "unit": "b", },
					{"name": "diskFree", "value": diskPartitionInfo["diskFree"], "unit": "b", },
					{"name": "diskReadIO", "value": diskPartitionInfo["diskReadIO"], "unit": "bps", },
					{"name": "diskWriteIO", "value": diskPartitionInfo["diskWriteIO"], "unit": "bps", },
					{"name": "diskRWAwait", "value": diskPartitionInfo["diskRWAwait"], "unit": "ms"},
					{"name": "diskReadAwait", "value": diskPartitionInfo["diskReadAwait"], "unit": "ms"},
					{"name": "diskWriteAwait", "value": diskPartitionInfo["diskWriteAwait"], "unit": "ms"},
					{"name": "diskReadOnly", "value": diskPartitionInfo["diskReadOnly"], "unit": "" },
					{"name": "diskInodesFree", "value": diskPartitionInfo["diskInodesFree"], "unit": "count"},
					{"name": "diskInodesTotal", "value": diskPartitionInfo["diskInodesTotal"], "unit": "count"},
					{"name": "diskIopsInProgress", "value": diskPartitionInfo["diskIopsInProgress"], "unit": "countps" },
					{"name": "diskReadIops", "value": diskPartitionInfo["diskReadIops"], "unit": "countps" },
					{"name": "diskWriteIops", "value": diskPartitionInfo["diskWriteIops"], "unit": "countps" },
					{"name": "diskWeightedIO", "value": diskPartitionInfo["diskWeightedIO"], "unit": "countps"},
					{"name": "diskDeviceName", "value": diskPartitionInfo["diskDeviceName"], "unit": "", },
					{"name": "diskSerialNumber", "value": diskPartitionInfo["diskSerialNumber"], "unit": "", },
				}}
			diskCollector.MetricHandler.AddMetric(&metricData)
		}
		for _, e := range diskCollector.MetricHandler.PopMetrics() {
			diskCollector.MetricQueue.EnQueue(e)
		}
	}
}

func (diskCollector *DiskCollector) GetCollectorTicker() *time.Ticker {
	return diskCollector.Ticker
}

func init() {
	RegisterPlugin("diskCollector", func() BaseCollector {
		return new(DiskCollector)
	})
}
