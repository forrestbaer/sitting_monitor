import fs from 'fs'
import _  from 'lodash'
import chalk from 'chalk'

(() => {
  const width = 88
  const log = console.log 

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
      let mins = (parseFloat(tss[0])*60)+parseFloat(tss[1])
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

    Object.keys(d).forEach(day => {
      console.log(chalk.gray('-------------------------'))
      console.log(' ' + chalk.whiteBright.bgGray(day))
      console.log(chalk.gray('-------------------------'))

      let tm = 0
      let ct = 0
      let lp = ''

      d[day].forEach(j => {
        if (j.position === 'sat' && (lp === 'stood' || lp === '')) {
          lp = 'sat'
          ct = j.minutes
        }
        if (j.position === 'stood' && (lp === 'sat' || lp === '')) {
          lp = 'stood'
          tm = tm + (j.minutes - ct)
        }
      })

      console.log(`sat for ${Math.floor(tm / 60)}hrs and ${tm % 60} minutes\n`)
    })
  })
})()

