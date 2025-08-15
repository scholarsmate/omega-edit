#!/usr/bin/env node
const fs = require('fs');
const path = require('path');
function walk(dir){
  for(const entry of fs.readdirSync(dir,{withFileTypes:true})){
    const full=path.join(dir,entry.name);
    if(entry.isDirectory()){
      walk(full);
    } else if(entry.name.endsWith('.js')){
      const cjs=full.slice(0,-3)+'.cjs';
      fs.renameSync(full,cjs);
      let code=fs.readFileSync(cjs,'utf8');
      code=code.replace(/sourceMappingURL=(.+)\.js\.map/,'sourceMappingURL=$1.cjs.map');
      fs.writeFileSync(cjs,code);
      const mapPath=full+'.map';
      if(fs.existsSync(mapPath)){
        const cjsMap=cjs+'.map';
        let map=fs.readFileSync(mapPath,'utf8');
        map=map.replace(/"file":"(.+?)\.js"/,'"file":"$1.cjs"');
        fs.renameSync(mapPath,cjsMap);
        fs.writeFileSync(cjsMap,map);
      }
    }
  }
}
const target=process.argv[2]||path.join('dist','cjs');
if(fs.existsSync(target)){
  walk(target);
}
