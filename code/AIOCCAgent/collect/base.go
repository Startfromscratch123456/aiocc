package collect

import (
	"net"
	"strings"
	"time"
	. "../metric"
	"../util"
)

type BaseCollectorFiled struct {
	CollectInterval int64
	Ticker          *time.Ticker
	MetricHandler   *MetricHandler
	MetricQueue     *MetricQueue
}

type BaseCollector interface {
	CollectorInit(metricQueue *MetricQueue)
	DoCollect()
	GetCollectorTicker() *time.Ticker
}

func GetVMUUID() string {
	return util.GetUUID()
}

func GetVMIP() string {
	server := "172.0.0.1:80"
	if conn, err := net.Dial("udp", server); err == nil {
		localAddr := conn.LocalAddr().String()
		return localAddr[0:strings.Index(localAddr, ":")]
	} else if addrs, err := net.InterfaceAddrs(); err == nil {
		for _, a := range addrs {
			if ipnet, ok := a.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
				if ipnet.IP.To4() != nil {
					return ipnet.IP.String()
				}
			}
		}
	}
	util.GetLogger().Println("获取ip信息失败")
	return ""
}

var (
	AvailablePlugins = make(map[string]func() BaseCollector)
)

func RegisterPlugin(name string, factory func() BaseCollector) {
	AvailablePlugins[name] = factory
}
