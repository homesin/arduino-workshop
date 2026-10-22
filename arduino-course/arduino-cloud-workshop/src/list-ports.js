import { SerialPort } from 'serialport';

const ports = await SerialPort.list();
if (ports.length === 0) {
  console.log('沒有偵測到任何序列埠。確認 USB 線插好，而且不是只能充電的線。');
} else {
  console.log('偵測到的連接埠：');
  for (const p of ports) {
    const hint = /wch|ch340|arduino|usb-serial/i.test(`${p.manufacturer} ${p.friendlyName}`) ? '  ← 很可能是這個' : '';
    console.log(`  ${p.path.padEnd(8)} ${(p.friendlyName || p.manufacturer || '').slice(0, 46)}${hint}`);
  }
  console.log('\n把上面的埠號填進 .env 的 SERIAL_PORT。');
}
