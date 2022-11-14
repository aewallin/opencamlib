const puppeteer = require('puppeteer');

(async () => {
  const browser = await puppeteer.launch();
  const page = await browser.newPage();
  await page.setDefaultNavigationTimeout(0);
  page.on('console', msg => console.log(msg.text()));
  await page.goto('http://localhost:8000', {
    waitUntil: 'networkidle0',
  });
  await browser.close();
})();
