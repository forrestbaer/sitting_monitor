(() => {
  const fs = require('fs')
  const _ = require('lodash')
  const width = 88

  fs.readFile('../../log/sitmon.log', 'utf8' , (err, data) => {
    if (err) {
      console.error(err)
      return
    }

    let d = data.split(/\r?\n/).map(i => {
      return i.split(',')
    })

    d = d.slice(0, d.length-1).map(i => { 
      let t = new Date(`${i[0]} ${i[1]}`).toLocaleTimeString()
      let ts = t.split(':')
      let tss = ts.slice(0, ts.length-1)
      let mins = (parseInt(tss[0])*60)+parseInt(tss[1])
      let percent = parseFloat(mins / 1440)

      return {
        time: t,
        minutes: mins,
        percent: percent,
        date: new Date(`${i[0]} ${i[1]}`).toDateString(),
        position: i[2],
      }
    })

    d = _.groupBy(d, 'date')

    Object.keys(d).forEach(k => {
      console.log(k)
      d[k].forEach(j => {
        let charpos = parseFloat(j.percent * width).toFixed(2)
        console.log(`${charpos} -- ${j.position}`)
      })
    })
  })
})()

