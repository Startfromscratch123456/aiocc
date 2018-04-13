package util

import (
	"../conf"
	"io/ioutil"
	"net/http"
	"time"
)

var UUID string

func UUIDInit() string {
	readUUID, err := ioutil.ReadFile(conf.GetConfigTool().UUIDFile)
	if err != nil {
		//请求
		client := http.Client{
			Timeout: time.Duration(time.Duration(conf.GetConfigTool().RequestDeadline) * time.Second),
		}
		resp, err := client.Get(conf.GetConfigTool().UUIDRequestURL)
		if err != nil {
			UUID = ""
			GetLogger().Println("请求获取uuid失败!")
			return UUID
		}
		body, err := ioutil.ReadAll(resp.Body)
		UUID = string(body)
		err = ioutil.WriteFile(conf.GetConfigTool().UUIDFile, []byte(UUID), 0666)
		if err != nil {
			UUID = ""
			GetLogger().Println("向" + conf.GetConfigTool().UUIDFile + "写入uuid信息失败!")
			return UUID
		}
	} else {
		UUID = string(readUUID)
	}
	return UUID
}
func GetUUID() string {
	return UUID
}
