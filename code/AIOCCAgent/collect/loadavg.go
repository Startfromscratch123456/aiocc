package collect

import (
	"fmt"
	"io/ioutil"
	"runtime"
	"errors"
)

type Loadavg struct {
	LoadAverage1     float64
	LoadAverage5     float64
	LoadAverage15    float64
	RunningProcesses int64
	TotalProcesses   int64
	LastProcessId    int64
}

func GetLoadAvg() (*Loadavg, error) {
	switch runtime.GOOS {
	case "linux":
		return parseLinux()
	default:
		return nil, errors.New("loadavg unimplemented on " + runtime.GOOS)
	}
}

func parseLinux() (*Loadavg, error) {
	self := new(Loadavg)

	raw, err := ioutil.ReadFile("/proc/loadavg")
	if err != nil {
		return self, err
	}

	fmt.Sscanf(string(raw), "%f %f %f %d/%d %d",
		&self.LoadAverage1, &self.LoadAverage5, &self.LoadAverage15,
		&self.RunningProcesses, &self.TotalProcesses,
		&self.LastProcessId)
	return self, nil
}
