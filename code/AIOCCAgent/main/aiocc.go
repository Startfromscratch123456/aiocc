package main

import (
	. "../collect"
	. "../metric"
	. "../dispatch"
	"../conf"
	"../util"
	"sync"
	"github.com/ShijunDeng/go-daemon"
	"flag"
)

func doInit(configFile string) {
	if conf.ConfigureInit(configFile) != nil {
		//配置文件初始化,如果这都失败,怎么处理？
	}
	if util.LoggerInit() != nil {
		//日志建立失败
	}
	util.UUIDInit()
}

func StartCollect() {
	metricQueue := &MetricQueue{}
	metricQueue.MetricQueueInit()

	wg := new(sync.WaitGroup)
	for _, baseCollector := range AvailablePlugins {
		wg.Add(1)
		go func(f func() BaseCollector) {
			v := f()
			v.CollectorInit(metricQueue)
			for true {
				select {
				case <-v.GetCollectorTicker().C:
					v.DoCollect()
				}
			}
			//wg.Done()
		}(baseCollector)
	}

	metricSender := &MetricSender{}
	metricSender.MetricSenderInit(metricQueue)

	go func() {
		wg.Add(1)
		metricSender.DoSend()
	}()

	wg.Wait()
}

func main() {

	configFile := flag.String("config", "conf/conf.toml", "configuration file")
	doInit(*configFile)

	cntxt := &daemon.Context{
		PidFileName: conf.GetConfigTool().PIDFile,
		PidFilePerm: 0644,
		LogFileName: conf.GetConfigTool().LogFilePath + conf.GetConfigTool().LogFileName,
		LogFilePerm: 0640,
		WorkDir:     "./",
		Umask:       027,
		Args:        []string{conf.GetConfigTool().AgentName},
	}
	d, err := cntxt.Reborn()
	if err != nil {
		util.GetLogger().Println(err.Error())
	}
	if d != nil {
		return
	}
	defer cntxt.Release()

	util.GetLogger().Println(cntxt.Args[0] + " started...")
	StartCollect()
}
