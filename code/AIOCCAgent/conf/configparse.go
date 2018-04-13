package conf

import (
	"github.com/ShijunDeng/toml"
	"os"
	"io/ioutil"
)

/*对应conf.toml*/
type Configure struct {
	AgentName              string
	ProcessList            []string
	LogFilePath            string
	UploadURL              string
	LogFileName            string
	LogPrefix              string
	CompressOP             bool
	UploadInterval         int64
	DiskCollectInterval    int64
	CpuCollectInterval     int64
	MemoryCollectInterval  int64
	TcpCollectInterval     int64
	UdpCollectInterval     int64
	NetCollectInterval     int64
	ProcessCollectInterval int64
	SystemCollectInterval  int64
	UUIDRequestURL         string
	UUIDFile               string
	PIDFile                string
	UPTIMEFile             string
	UploadDeadline         int64
	RequestDeadline        int64
}

var cfg = &Configure{}

func ConfigureInit(configFile string) (err error) {
	var (
		fp       *os.File
		confByte []byte
	)

	if fp, err = os.Open(configFile); err != nil {
		return err
	}

	if confByte, err = ioutil.ReadAll(fp); err != nil {
		return err
	}

	if err = toml.Unmarshal(confByte, cfg); err != nil {
		return err
	}
	return nil
}
func GetConfigTool() *Configure {
	return cfg
}
