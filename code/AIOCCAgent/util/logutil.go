package util

import (
	"log"
	"os"
	"../conf"
)

/*

func LogToFile(logStr string) {
	logfile, err := os.OpenFile(conf.GetConfigTool().LogFilePath+conf.GetConfigTool().LogFileName, os.O_RDWR|os.O_CREATE|os.O_APPEND, 0666)
	if err != nil {
		//处理,发条信息到服务器吧
	} else {
		logger := log.New(logfile, conf.GetConfigTool().LogPrefix, log.Ldate|log.Ltime|log.Llongfile)
		logger.Println(logStr)
		defer logfile.Close()
	}
}
*/

var logger *log.Logger

func LoggerInit() (error) {
	logfile, err := os.OpenFile(conf.GetConfigTool().LogFilePath+conf.GetConfigTool().LogFileName, os.O_RDWR|os.O_CREATE|os.O_APPEND, 0666)
	if err != nil {
		logfile, err = os.OpenFile("./tmplog.log", os.O_RDWR|os.O_CREATE|os.O_APPEND, 0666)
		//处理,发条信息到服务器吧
		if err != nil {
			return err
		}
	}
	logger = log.New(logfile, conf.GetConfigTool().LogPrefix, log.Ldate|log.Ltime|log.Llongfile)
	return nil
}

func GetLogger() (*log.Logger) {
	return logger
}
