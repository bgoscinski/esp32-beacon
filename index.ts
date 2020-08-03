const IP = '192.168.1.254'
const CMD_PORT = 80
const HEARTBEAT_PORT = 3333
const HEARTBEAT_MSG = new TextEncoder().encode('02:001:0')

const delay = (ms: number) => new Promise((resolve) => setTimeout(resolve, ms))

const Cmd = (cmd: number, par?: number) => () =>
  par === undefined
    ? fetch(`http://${IP}:${CMD_PORT}/?custom=1&cmd=${cmd}`)
    : fetch(`http://${IP}:${CMD_PORT}/?custom=1&cmd=${cmd}&par=${par}`)

const Get = (path: string) => fetch(`http://${IP}:${CMD_PORT}/${path}`)
const Del = (path: string) =>
  fetch(`http://${IP}:${CMD_PORT}/${path}?custom=1&cmd=4003`)

const yi_cardInfo = Cmd(3039)
const yi_clock = Cmd(3034)
const yi_config = Cmd(3014)
const yi_connect = Cmd(8001)
const yi_disconnect = Cmd(8002)
const yi_fileDelete = Cmd(4003)
const yi_fileForceDelete = Cmd(4009)
const yi_fileList = Cmd(3015)
const yi_fileThumbnail = Cmd(4001)
const yi_modePhoto = Cmd(3001, 0)
const yi_modeVideo = Cmd(3001, 1)
const yi_modeFile = Cmd(3001, 2)
const yi_takePhoto = Cmd(1001)
const yi_videoEmergency = Cmd(2019)
const yi_videoPhoto = Cmd(2017)
const yi_videoRecordOff = Cmd(2001, 0)
const yi_videoRecordOn = Cmd(2001, 1)
const yi_videoSecondsLeft = Cmd(2009)
const yi_videoState = Cmd(2016)
const yi_videoStreamOff = Cmd(2015, 0)
const yi_videoStreamOn = Cmd(2015, 1)

const yi_adas = Cmd(2031)
const yi_audio = Cmd(2007)
const yi_buttonSound = Cmd(3041)
const yi_drivingReport = Cmd(3044)
const yi_exposure = Cmd(2005)
const yi_firmwareVersion = Cmd(3012)
const yi_gsensor = Cmd(2011)
const yi_language = Cmd(3008)
const yi_model = Cmd(3035)
const yi_photoResolution640 = Cmd(1002, 5)
const yi_photoResolution1280 = Cmd(1002, 6)
const yi_photoResolution1920 = Cmd(1002, 7)
const yi_photoResolution2048 = Cmd(1002, 4)
const yi_photoResolution2592 = Cmd(1002, 3)
const yi_photoResolution3264 = Cmd(1002, 2)
const yi_photoResolution3648 = Cmd(1002, 1)
const yi_photoResolution4032 = Cmd(1002, 0)
const yi_powerOnOffSound = Cmd(2051)
const yi_serialNumber = Cmd(3037)
const yi_standbyClock = Cmd(2050)
const yi_standbyTimeout = Cmd(3033)
const yi_videoAutoStartOff = Cmd(2012, 0)
const yi_videoAutoStartOn = Cmd(2012, 1)
const yi_videoLength = Cmd(2003)
const yi_videoLogo = Cmd(2040)
const yi_videoResolution = Cmd(2002)
const yi_videoTimestamp = Cmd(2008)

let connected = false
let hbProc: number
async function connect() {
  const res = await yi_connect()
  const hbConn = await Deno.connect({ hostname: IP, port: HEARTBEAT_PORT })
  console.log('Connected!')

  connected = true
  ;(function loop() {
    hbProc = setTimeout(() => {
      hbConn.write(HEARTBEAT_MSG).then(loop).catch(console.error)
    }, 5000)
  })

  return res
}

async function disconnect() {
  const res = await yi_disconnect()
  clearTimeout(hbProc)
  connected = false
  return res
}

while (true) {
  try {
    await connect()
    break
  } catch {}
}

await logResponse('videoRecordOff', yi_videoRecordOff)
await logResponse('modeVideo', yi_modeVideo)
await logResponse('videoRecordOff', yi_videoRecordOff)
await logResponse('yi_videoAutoStartOff', yi_videoAutoStartOn)
await logResponse('photoResolution1920', yi_photoResolution1920)
await delay(1)
await logResponse('modePhoto', yi_modePhoto)
await delay(1)

const res = await logResponse('takePhoto', yi_takePhoto)
const path = res.match(/<FPATH>A:\\(.*?)<\/FPATH/)![1].replace(/\\/g, '/')
await logResponse('modeFile', yi_modeFile)
await fetch(`http://${IP}:${CMD_PORT}/${path}`)
  .then((r) => r.arrayBuffer())
  .then((file) => Deno.writeFile('photo.jpg', new Uint8Array(file)))
await Del(path)
await logResponse('fileList', yi_fileList)
await disconnect()

async function logResponse(name: string, cmd: () => Promise<Response>) {
  console.log('\n\nSend command:', name)
  const res = await cmd()
  console.log(res.status, res.statusText)
  const text = await res.text()
  console.log(text)
  return text
}

yi_modeVideo
delay(2)
yi_videoRecordOff
delay(1)
