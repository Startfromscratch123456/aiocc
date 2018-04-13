package server

import (
	"encoding/json"
	"fmt"
	"net/http"
	"time"
	"bytes"
	"compress/zlib"
	"io"
	"strconv"
	"strings"
	"os"
	"sync"
)

type Metric struct {
	Dimensions map[string]string `json:"Dimensions"`
	Values     []map[string]string `json:"Values"`
}

type FileBanlanceManager struct {
	LastTimeStr string
	MaxSize     int64
	Suffix      string
	Port        string
	Pattern     string
	curFile     *os.File
}

type aioccServer struct {
	FBManager *FileBanlanceManager
}

func (f *FileBanlanceManager) GetFileName() (string) {
	return "/var/log/collected_data/" + f.LastTimeStr + "_" + f.Suffix
}
func (f *FileBanlanceManager) FileBanlanceManagerInit(prefix string, maxSize int64) (error) {
	f.LastTimeStr = time.Now().Format("20060102030405")
	f.MaxSize = maxSize
	f.Suffix = prefix
	var err error
	f.curFile, err = os.OpenFile(f.GetFileName(), os.O_CREATE|os.O_RDWR|os.O_APPEND, 0666)
	if err == nil {
		f.curFile.Truncate(0)
	}
	return err

}

func (f *FileBanlanceManager) GetMasterFile() (*os.File, error) {
	if stat, err := f.curFile.Stat(); err == nil {
		if stat.Size() > f.MaxSize {
			if err := f.curFile.Close(); err == nil {
				f.LastTimeStr = time.Now().Format("20060102030405")
				if truncFile, err := os.OpenFile(f.GetFileName(), os.O_CREATE|os.O_RDWR|os.O_APPEND, 0666); err == nil {
					truncFile.Truncate(0)
					f.curFile = truncFile
					return f.curFile, nil
				} else {
					return nil, err
				}
			} else {
				return nil, err
			}
		}
	} else {
		return nil, err
	}
	return f.curFile, nil
}

func DoZlibUnCompress(compressSrc []byte) []byte {
	b := bytes.NewReader(compressSrc)
	var out bytes.Buffer
	r, _ := zlib.NewReader(b)
	io.Copy(&out, r)
	return out.Bytes()
}
func (aioccServer *aioccServer) aioccServerInit(fileBanlanceManager *FileBanlanceManager) {
	aioccServer.FBManager = fileBanlanceManager
}
func (aioccServer *aioccServer) ParseGhPost(rw http.ResponseWriter, request *http.Request) {
	if request.Body == nil {
		fmt.Println("nil")
		return
	}
	var err error
	ret := &Metric{}
	length, _ := strconv.Atoi(request.Header.Get("Content-Length"))
	p := make([]byte, length)
	request.Body.Read(p)
	pp := make([]byte, length)
	if strings.Compare(request.Header.Get("Content-Encoding"), "gzip") == 0 {
		pp = DoZlibUnCompress(p)
		fmt.Printf("zip=%v ungizp%v\n", len(p), len(pp))
	} else {
		pp = p
	}
	err = json.Unmarshal(pp, ret)
	if err != nil {
		fmt.Println(err)
	}

	fmt.Printf("timestamp=[%v],received \n%v\n", time.Now(), *ret)
	n, err := aioccServer.appendToFile(string(pp) + "\n")
	fmt.Printf("write to file:file=%v bytes=%v err=%v\n\n\n", aioccServer.FBManager.curFile.Name(), n, err)
}

func (aioccServer *aioccServer) appendToFile(content string) (int, error) {
	f, err := aioccServer.FBManager.GetMasterFile()
	if err != nil {
		fmt.Println("cacheFileList.yml file create failed. err: " + err.Error())
	} else {
		n, err := f.WriteString(content)
		return n, err

	}
	return 0, err
}

func main() {
	sDIdi := &FileBanlanceManager{Suffix: "didicloud", MaxSize: 16 * 1024 * 1024, Port: ":8080", Pattern: "/didi"}
	sDIdi.FileBanlanceManagerInit("didicloud", 16*1024*1024)
	sTenxun := &FileBanlanceManager{Suffix: "tenxuncloud", MaxSize: 16 * 1024 * 1024, Port: ":8081", Pattern: "/tenxun"}
	sTenxun.FileBanlanceManagerInit("tenxuncloud", 16*1024*1024)
	servers := map[string]aioccServer{
		"sDIdi":   {FBManager: sDIdi},
		"sTenxun": {FBManager: sTenxun}, }

	wg := new(sync.WaitGroup)
	for _, server := range servers {
		wg.Add(1)
		go func(server aioccServer) {
			http.HandleFunc(server.FBManager.Pattern, server.ParseGhPost)
			http.ListenAndServe(server.FBManager.Port, nil)
		}(server)
	}
	wg.Wait()
}
