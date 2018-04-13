package dispatch

import (
	"encoding/json"
	. "../metric"
	"../conf"
	"bytes"
	"net/http"
	"io/ioutil"
	"strconv"
	"compress/zlib"
	"io"
	"time"
	"../util"
	"errors"
)

type MetricSender struct {
	URL                string
	CompressOP         bool
	UploadInterval     int64
	StopChan           chan bool
	MetricSenderTicker *time.Ticker
	MetricQueue        *MetricQueue
}

//进行zlib压缩
func DoZlibCompress(src []byte) []byte {
	var in bytes.Buffer
	w := zlib.NewWriter(&in)
	w.Write(src)
	w.Flush()
	defer w.Close()
	return in.Bytes()
}

//进行zlib解压缩
func DoZlibUnCompress(compressSrc []byte) ([]byte, error) {
	b := bytes.NewReader(compressSrc)
	var out bytes.Buffer
	r, err := zlib.NewReader(b)
	if err == nil {
		io.Copy(&out, r)
		return out.Bytes(), nil
	}
	util.GetLogger().Println(err.Error())
	return nil, err
}

func (metricSender *MetricSender) MetricSenderInit(metricQueue *MetricQueue) {
	metricSender.URL = conf.GetConfigTool().UploadURL
	metricSender.CompressOP = conf.GetConfigTool().CompressOP
	metricSender.UploadInterval = int64(conf.GetConfigTool().UploadInterval)
	metricSender.MetricSenderTicker = time.NewTicker(time.Duration(conf.GetConfigTool().UploadInterval) * time.Second)
	metricSender.MetricQueue = metricQueue
	metricSender.StopChan = make(chan bool)
}

func (metricSender *MetricSender) DoSend() {
	loop := true
	for loop {
		select {
		case metric := <-metricSender.MetricQueue.MetricChan:
			if metric != nil {
				metricSender.doSend(*metric)
			}
		case <-metricSender.StopChan:
			loop = false
		}
	}
}

func (metricSender *MetricSender) doSend(metrics Metric) (error) {
	tmpByte, err := json.Marshal(metrics)

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	//fmt.Printf("发送数据:%v\n", metrics)
	if metricSender.CompressOP {
		tmpByte = DoZlibCompress(tmpByte)
	}

	request, err := http.NewRequest("POST", metricSender.URL, bytes.NewBuffer(tmpByte))

	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	}

	request.Header.Set("Content-Length", strconv.Itoa(len(tmpByte)))
	request.Header.Set("Content-Type", "application/json")

	if metricSender.CompressOP {
		request.Header.Set("Content-Encoding", "gzip")
	}

	client := http.Client{
		Timeout: time.Duration(conf.GetConfigTool().RequestDeadline) * time.Second,
	}
	response, err := client.Do(request)

	defer request.Body.Close()
	if err != nil {
		util.GetLogger().Println(err.Error())
		return err
	} else if response.StatusCode != 200 {
		body, _ := ioutil.ReadAll(response.Body)
		errStr := string(body)
		util.GetLogger().Println(errStr)
		return errors.New(errStr)
	}
	return nil
}

func (metricSender *MetricSender) DoStop() {
	metricSender.StopChan <- false
}
