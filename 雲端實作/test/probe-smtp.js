/*
 * 探測一台 SMTP 伺服器：它在不在、走哪個埠、支不支援 STARTTLS、要不要認證。
 *
 *   node test/probe-smtp.js                        探測 ncyu.edu.tw 的常見主機名
 *   node test/probe-smtp.js smtp.example.com       指定主機
 *   node test/probe-smtp.js a.com b.com            一次探測多台
 *
 * 這支**不會寄信、不會登入**，只跟伺服器打個招呼（EHLO）看它怎麼回答。
 * 用來在問資訊中心之前，先自己確認哪個主機名、哪個埠是通的。
 */
import net from 'node:net';
import tls from 'node:tls';
import dns from 'node:dns/promises';

const DEFAULT_DOMAIN = 'ncyu.edu.tw';
const PORTS = [587, 465, 25];
const TIMEOUT_MS = 6000;

/** 跟伺服器完成一次 greeting + EHLO，回傳它宣告的能力。 */
function probe(host, port) {
  return new Promise((resolve) => {
    const secure = port === 465; // 465 一連上就是 TLS，587 與 25 是明文起頭
    const chunks = [];
    let settled = false;

    const finish = (result) => {
      if (settled) return;
      settled = true;
      try { socket.destroy(); } catch {}
      resolve(result);
    };

    const socket = (secure ? tls : net).connect(
      secure ? { host, port, servername: host, rejectUnauthorized: false } : { host, port },
      () => socket.write(`EHLO probe.local\r\n`)
    );

    socket.setTimeout(TIMEOUT_MS);
    socket.setEncoding('utf-8');

    socket.on('data', (chunk) => {
      chunks.push(chunk);
      const text = chunks.join('');
      // EHLO 的回應最後一行是「250 」（空格，不是減號）
      if (/^250 [^\n]*\r?\n$/m.test(text) || /\n250 [^\n]*\r?\n/.test(text)) {
        const lines = text.split(/\r?\n/).filter(Boolean);
        const caps = lines
          .filter((l) => /^250[ -]/.test(l))
          .map((l) => l.slice(4).trim())
          .filter(Boolean);
        finish({
          host,
          port,
          reachable: true,
          greeting: lines[0] ?? '',
          starttls: caps.some((c) => /^STARTTLS/i.test(c)),
          auth: caps.find((c) => /^AUTH/i.test(c)) ?? '',
          caps,
        });
      }
    });

    socket.on('timeout', () => finish({ host, port, reachable: false, error: '逾時（多半是被防火牆擋掉）' }));
    socket.on('error', (err) => finish({ host, port, reachable: false, error: err.code || err.message }));
  });
}

const args = process.argv.slice(2);
let hosts = args;

if (hosts.length === 0) {
  console.log('');
  console.log(`  查 ${DEFAULT_DOMAIN} 的 MX 記錄……`);
  let mx = [];
  try {
    mx = (await dns.resolveMx(DEFAULT_DOMAIN)).sort((a, b) => a.priority - b.priority);
    for (const r of mx) console.log(`    MX ${r.priority}  ${r.exchange}`);
  } catch (err) {
    console.log(`    查不到（${err.code}）`);
  }
  console.log('');
  // MX 是「收信」主機，送信中繼常常是另一台，所以兩邊都試
  hosts = [...new Set([
    ...mx.map((r) => r.exchange.replace(/\.$/, '')),
    `smtp.${DEFAULT_DOMAIN}`,
    `mail.${DEFAULT_DOMAIN}`,
    `msa.${DEFAULT_DOMAIN}`,
    `smtp.mail.${DEFAULT_DOMAIN}`,
  ])];
}

console.log(`  探測 ${hosts.length} 台主機 × ${PORTS.length} 個埠，每個最多等 ${TIMEOUT_MS / 1000} 秒……`);
console.log('');

for (const host of hosts) {
  // 先確認這個名字解得出 IP，省得對不存在的主機空等三次
  try {
    const { address } = await dns.lookup(host);
    console.log(`  ${host}  →  ${address}`);
  } catch {
    console.log(`  ${host}  →  查不到這個主機名`);
    console.log('');
    continue;
  }

  const results = await Promise.all(PORTS.map((port) => probe(host, port)));
  for (const r of results) {
    if (!r.reachable) {
      console.log(`      ${String(r.port).padEnd(5)} ✘  ${r.error}`);
      continue;
    }
    // 465 本來就是「一連上就加密」，它的 EHLO 裡不會有 STARTTLS，那是正常的，
    // 不要顯示成 ✘ 讓人誤以為不安全。
    const encryption = r.port === 465
      ? '隱式 TLS（連線本身已加密）'
      : r.starttls ? 'STARTTLS ✔' : 'STARTTLS ✘（密碼會以明文傳送，不要用）';
    const bits = [encryption, r.auth ? `認證：${r.auth}` : '不需要認證（或未公開）'];
    console.log(`      ${String(r.port).padEnd(5)} ✔  ${bits.join('　')}`);
    console.log(`            ${r.greeting.slice(0, 90)}`);
  }
  console.log('');
}

console.log('  怎麼看結果：');
console.log('    587 通且有 STARTTLS 與 AUTH → 最理想，.env 就填這台、SMTP_PORT=587');
console.log('    465 通                      → 也可以，SMTP_PORT=465（程式會自動走 SSL）');
console.log('    只有 25 通且不需認證         → 通常只讓校內網路用，且只能寄給校內信箱');
console.log('    全部逾時                    → 你的網路擋住對外 SMTP，要問資訊中心');
console.log('');
