const searcher = require('./build/Release/searcher');
const util = require('util')

const findByJSON = util.promisify(searcher.findByJSONLocal)
const findByHandle = util.promisify(searcher.findByHandleLocal)
const findBySiteMap = util.promisify(searcher.findBySiteMap)

async function test(domain, pKeywords, nKeywords, size, jwt_token) {
  try {
    let p = await findBySiteMap(domain, "154.3.195.28:1337:tfuj4bw:a2803", pKeywords, nKeywords, size, false, jwt_token);
    console.log(p);
  } catch (err) {
    console.log(err)
  }

}

const domain = "kextest.myshopify.com";
const positiveKW = ["yeezy"];
const negativeKW = [];
const size = "random"
let token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiI1Y2U1NjI1NjUzMDE4MzAwMWVhMmRjMGEiLCJsaWNlbnNlS2V5IjoiUFpNVjgtWlRDVlgtVzRVNFktOVFBV1gtOEIyWFMiLCJod2lkIjoiY2Y0OGU0ZDMtNWFhZC00NGQwLWI5YTYtYzFmNDRlMzJkZTg3IiwiaWF0IjoxNTYyNDcxODgxLCJleHAiOjk5OTk5OTk5OTl9.Nnl0VfBYRf1QVdPYhb8x4j2DBpFBxJR8ynIg99abwHg"

test(domain, positiveKW, negativeKW, size, token)
