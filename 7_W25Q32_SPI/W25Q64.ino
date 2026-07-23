#include <Arduino.h>
#include <SPI.h>
#include <string.h>

// -------------------------- 引脚定义 --------------------------
constexpr int PIN_FLASH_CS = 10;
constexpr int PIN_FLASH_SCK = 12;
constexpr int PIN_FLASH_MOSI = 11;
constexpr int PIN_FLASH_MISO = 13;

// -------------------------- W25Q64 参数 --------------------------
constexpr uint32_t
FLASH_TOTAL_SIZE = 8UL * 1024UL * 1024UL;
constexpr uint32_t
FLASH_PAGE_SIZE = 256;
constexpr uint32_t
FLASH_SECTOR_SIZE = 4096;
constexpr uint32_t
STORE_ADDR = 0x001000;
constexpr size_t
MAX_STR_LEN = 512;
constexpr uint32_t
SPI_SPEED = 1000000;

// -------------------------- W25Q 指令 --------------------------
constexpr uint8_t
CMD_WRITE_EN = 0x06;
constexpr uint8_t
CMD_RD_STATUS = 0x05;
constexpr uint8_t
CMD_READ_DATA = 0x03;
constexpr uint8_t
CMD_PAGE_WR = 0x02;
constexpr uint8_t
CMD_SEC_ERASE = 0x20;
constexpr uint8_t
CMD_JEDEC_ID = 0x9F;

// -------------------------- 存储头部 --------------------------
constexpr uint32_t
MAGIC_TAG = 0x57323536UL;
constexpr uint16_t
DATA_VER = 1;

struct __attribute__((packed)) StorageHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t strLen;
    uint32_t strCrc;
    uint32_t headCrc;
};
static_assert(sizeof(StorageHeader) == 16, "Header must be 16 Bytes");

// -------------------------- 全局变量 --------------------------
const SPISettings spiFlashCfg(SPI_SPEED, MSBFIRST, SPI_MODE0);
bool flashReady = false;
char uartInputBuf[MAX_STR_LEN + 1] = {0};
size_t uartInputLen = 0;

// -------------------------- SPI基础操作 --------------------------
void flashCS_Select() { digitalWrite(PIN_FLASH_CS, LOW); }

void flashCS_Deselect() { digitalWrite(PIN_FLASH_CS, HIGH); }

void send24Addr(uint32_t
addr)
{
SPI.
transfer((uint8_t)(addr >> 16)
);
SPI.
transfer((uint8_t)(addr >> 8)
);
SPI.
transfer((uint8_t)
addr);
}

uint32_t calcCRC32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFUL;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320UL;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFFUL;
}

uint32_t readJEDEC() {
    SPI.beginTransaction(spiFlashCfg);
    flashCS_Select();
    SPI.transfer(CMD_JEDEC_ID);
    uint8_t man = SPI.transfer(0xFF);
    uint8_t memType = SPI.transfer(0xFF);
    uint8_t cap = SPI.transfer(0xFF);
    flashCS_Deselect();
    SPI.endTransaction();
    return ((uint32_t)
    man << 16) | ((uint32_t)
    memType << 8) | cap;
}

uint8_t readStatusReg() {
    SPI.beginTransaction(spiFlashCfg);
    flashCS_Select();
    SPI.transfer(CMD_RD_STATUS);
    uint8_t sta = SPI.transfer(0xFF);
    flashCS_Deselect();
    SPI.endTransaction();
    return sta;
}

bool waitFlashReady(uint32_t
timeoutMs)
{
uint32_t start = millis();
while(

readStatusReg() &

0x01)
{
if(

millis()

- start >= timeoutMs)
return false;
delay(1);
}
return true;
}

bool writeEnable() {
    SPI.beginTransaction(spiFlashCfg);
    flashCS_Select();
    SPI.transfer(CMD_WRITE_EN);
    flashCS_Deselect();
    SPI.endTransaction();
    return (readStatusReg() & 0x02) != 0;
}

bool erase4KSector(uint32_t
addr)
{
uint32_t secAddr = addr & (~(FLASH_SECTOR_SIZE - 1));
if(!

writeEnable()

)
{
Serial.println("[ERR] 擦除写使能失败");
return false;
}
SPI.
beginTransaction(spiFlashCfg);

flashCS_Select();

SPI.
transfer(CMD_SEC_ERASE);
send24Addr(secAddr);

flashCS_Deselect();

SPI.

endTransaction();

if(!waitFlashReady(5000))
{
Serial.println("[ERR] 擦除超时");
return false;
}
return true;
}

bool pageWrite(uint32_t
addr,
const uint8_t *data, size_t
len)
{
if(!data || len == 0) return false;
uint32_t pageOff = addr & (FLASH_PAGE_SIZE - 1);
size_t pageRemain = FLASH_PAGE_SIZE - pageOff;
if(len > pageRemain)
{
Serial.println("[ERR] 写入跨页");
return false;
}
if(!

writeEnable()

)
{
Serial.println("[ERR] 写使能失败");
return false;
}
SPI.
beginTransaction(spiFlashCfg);

flashCS_Select();

SPI.
transfer(CMD_PAGE_WR);
send24Addr(addr);
for(
size_t i = 0;
i<len;
i++)
SPI.
transfer(data[i]);

flashCS_Deselect();

SPI.

endTransaction();

if(!waitFlashReady(1000))
{
Serial.println("[ERR] 页写入超时");
return false;
}
return true;
}

bool flashWriteData(uint32_t
addr,
const uint8_t *data, size_t
len)
{
if(len == 0) return true;
uint32_t curAddr = addr;
const uint8_t *curData = data;
size_t curLen = len;
while(curLen > 0)
{
uint32_t off = curAddr & (FLASH_PAGE_SIZE - 1);
size_t chunk = FLASH_PAGE_SIZE - off;
if(chunk > curLen)
chunk = curLen;
if(!
pageWrite(curAddr, curData, chunk
))
return false;
curAddr +=
chunk;
curData +=
chunk;
curLen -=
chunk;
}
return true;
}

bool flashReadData(uint32_t
addr,
uint8_t *buf, size_t
len)
{
if(len == 0) return true;
if(!buf) return false;
if(!waitFlashReady(1000))
{
Serial.println("[ERR] Flash忙，读取失败");
return false;
}
SPI.
beginTransaction(spiFlashCfg);

flashCS_Select();

SPI.
transfer(CMD_READ_DATA);
send24Addr(addr);
for(
size_t i = 0;
i<len;
i++)
buf[i] = SPI.transfer(0xFF);

flashCS_Deselect();

SPI.

endTransaction();

return true;
}

// 新增：两次读取Header，内容完全一致才可信
bool readStableHeader(StorageHeader &hdr) {
    StorageHeader h1, h2;
    memset(&h1, 0, sizeof(h1));
    memset(&h2, 0, sizeof(h2));
    if (!flashReadData(STORE_ADDR, (uint8_t * ) & h1, sizeof(h1))) return false;
    delay(5);
    if (!flashReadData(STORE_ADDR, (uint8_t * ) & h2, sizeof(h2))) return false;
    if (memcmp(&h1, &h2, sizeof(h1)) != 0) {
        Serial.println("[WARN] 两次读取头部不一致，读取不稳定");
        return false;
    }
    hdr = h1;
    return true;
}

// -------------------------- 存储逻辑（修复：先占位，最后写有效Header） --------------------------
bool saveStringToFlash(const char *str, size_t strLen) {
    if (!str || strLen == 0) {
        Serial.println("[WARN] 空文本不存储");
        return false;
    }
    if (strLen > MAX_STR_LEN) {
        Serial.println("[ERR] 文本超长");
        return false;
    }

    StorageHeader blankHdr{};
    memset(&blankHdr, 0xFF, sizeof(blankHdr)); // 空白无效头部占位

    StorageHeader validHdr{};
    validHdr.magic = MAGIC_TAG;
    validHdr.version = DATA_VER;
    validHdr.strLen = (uint16_t)
    strLen;
    validHdr.strCrc = calcCRC32((
    const uint8_t*)str, strLen);
    validHdr.headCrc = 0;
    validHdr.headCrc = calcCRC32((uint8_t * ) & validHdr, sizeof(StorageHeader));

    Serial.println("\n[1] 擦除4K存储扇区");
    if (!erase4KSector(STORE_ADDR)) return false;

    // 第一步：写入空白无效头部，中途断电永远不会读到有效数据
    Serial.println("[2] 写入空白占位头部");
    if (!flashWriteData(STORE_ADDR, (uint8_t * ) & blankHdr, sizeof(blankHdr))) return false;

    // 第二步：写入文本数据
    uint32_t dataAddr = STORE_ADDR + sizeof(StorageHeader);
    Serial.println("[3] 写入文本内容");
    if (!flashWriteData(dataAddr, (uint8_t * )str, strLen)) return false;

    // 第三步：全部写完，最后写入有效校验头部（关键修复点）
    Serial.println("[4] 写入有效校验头部（提交数据）");
    if (!flashWriteData(STORE_ADDR, (uint8_t * ) & validHdr, sizeof(validHdr))) return false;

    // 实时回读校验
    StorageHeader checkHead{};
    char checkBuf[MAX_STR_LEN + 1] = {0};
    if (!readStableHeader(checkHead)) return false;
    if (!flashReadData(dataAddr, (uint8_t * )checkBuf, strLen)) return false;

    uint32_t saveHdrCrc = checkHead.headCrc;
    checkHead.headCrc = 0;
    uint32_t calcHdrCrc = calcCRC32((uint8_t * ) & checkHead, sizeof(checkHead));
    if (calcHdrCrc != saveHdrCrc) {
        Serial.println("[ERR] 头部CRC校验失败");
        return false;
    }
    uint32_t
            calcStrCrc = calcCRC32((uint8_t * )
    checkBuf, strLen);
    if (calcStrCrc != validHdr.strCrc) {
        Serial.println("[ERR] 文本CRC校验失败");
        return false;
    }

    Serial.println("✅ 写入校验完成，实时读取：");
    Serial.println(checkBuf);
    return true;
}

bool loadStringFromFlash(char *outBuf, size_t bufSize, size_t &outLen) {
    outLen = 0;
    memset(outBuf, 0x00, bufSize);
    if (!outBuf || bufSize == 0) return false;

    StorageHeader head{};
    // 两次稳定读取头部
    if (!readStableHeader(head)) {
        Serial.println("[WARN] 头部读取不稳定，无有效数据");
        return false;
    }

    // 魔数校验
    if (head.magic != MAGIC_TAG) return false;
    if (head.version != DATA_VER) {
        Serial.println("[WARN] 数据版本不匹配");
        return false;
    }
    if (head.strLen == 0 || head.strLen > MAX_STR_LEN) {
        Serial.println("[WARN] 存储长度非法");
        return false;
    }
    if ((size_t)head.strLen + 1 > bufSize)
    {
        Serial.println("[ERR] 缓冲区空间不足");
        return false;
    }

    // 校验头部自身CRC
    uint32_t savedHdrCrc = head.headCrc;
    head.headCrc = 0;
    uint32_t calcHdrCrc = calcCRC32((uint8_t * ) & head, sizeof(head));
    if (calcHdrCrc != savedHdrCrc) {
        Serial.println("[WARN] 头部数据损坏");
        return false;
    }

    // 读取文本
    uint32_t dataAddr = STORE_ADDR + sizeof(StorageHeader);
    if (!flashReadData(dataAddr, (uint8_t * )outBuf, head.strLen))
    {
        Serial.println("[ERR] 读取文本失败");
        return false;
    }
    outBuf[head.strLen] = '\0';

    // 校验文本CRC
    uint32_t
            calcStrCrc = calcCRC32((uint8_t * )
    outBuf, head.strLen);
    if (calcStrCrc != head.strCrc) {
        Serial.println("[WARN] 文本数据损坏");
        outBuf[0] = '\0';
        return false;
    }

    outLen = head.strLen;
    return true;
}

void printSavedContent() {
    char textBuf[MAX_STR_LEN + 1] = {0};
    size_t dataLen = 0;

    if (loadStringFromFlash(textBuf, sizeof(textBuf), dataLen)) {
        Serial.println("\n==============================");
        Serial.println("上电读取存储内容：");
        Serial.println(textBuf);
        Serial.print("数据长度：");
        Serial.print(dataLen);
        Serial.println(" Byte");
        Serial.println("==============================\n");
    } else {
        Serial.println("\nFlash不存在完整有效存储数据\n");
    }
}

void processUartLine() {
    if (uartInputLen == 0) return;
    uartInputBuf[uartInputLen] = '\0';

    Serial.print("【收到输入文本】：");
    Serial.println(uartInputBuf);

    if (saveStringToFlash(uartInputBuf, uartInputLen))
        Serial.println("断电重启可正常读取！");
    else
        Serial.println("❌ 保存失败");

    memset(uartInputBuf, 0, sizeof(uartInputBuf));
    uartInputLen = 0;
    Serial.println("\n请输入新文本，回车保存：");
}

void setup() {
    Serial.begin(115200);
    delay(1200);

    pinMode(PIN_FLASH_CS, OUTPUT);
    flashCS_Deselect();

    SPI.begin(PIN_FLASH_SCK, PIN_FLASH_MISO, PIN_FLASH_MOSI, PIN_FLASH_CS);
    delay(100);

    uint32_t jedec = readJEDEC();
    Serial.print("W25Q JEDEC ID = 0x");
    Serial.println(jedec, HEX);

    if (jedec != 0xEF4017UL) {
        Serial.println("❌ 未识别W25Q64，检查3.3V供电与GND");
        flashReady = false;
        return;
    }
    flashReady = true;
    Serial.println("✅ W25Q64 芯片识别正常");

    printSavedContent();
    Serial.println("输入文字，回车存入Flash：");
}

void loop() {
    if (!flashReady) {
        delay(1000);
        return;
    }

    while (Serial.available() > 0) {
        char ch = Serial.read();
        if (ch == '\r') continue;
        if (ch == '\n') {
            processUartLine();
            continue;
        }
        if (uartInputLen < MAX_STR_LEN) {
            uartInputBuf[uartInputLen++] = ch;
        } else {
            Serial.println("\n⚠️ 输入超长，清空缓冲区");
            memset(uartInputBuf, 0, sizeof(uartInputBuf));
            uartInputLen = 0;
            while (Serial.available())
                if (Serial.read() == '\n') break;
        }
    }
}