const char INDEX_HTML[] = R"=====(
<html lang="de">
    <head>
        <link rel="stylesheet" href="style.css">
        <link rel="icon" type="image/svg" href="logo.svg">
        <meta charset="utf-8">
        <title>Zeit Messung</title>
    </head>
    <body>
        <header>
            <nav>
                <div id="navCenter">
                    <a touchstart="this.classList.add('navHover');" touchend="this.classList.remove('navHover');" onmouseover="this.classList.add('navHover');" onmouseout="this.classList.remove('navHover');" class="navLink" href="#">View</a>
                    <a touchstart="this.classList.add('navHover');" touchend="this.classList.remove('navHove');" onmouseover="this.classList.add('navHover');" onmouseout="this.classList.remove('navHover');" class="navLink" href="/wifiroot">WiFi</a>
                    <a touchstart="this.classList.add('navHover');" touchend="this.classList.remove('navHover');" onmouseover="this.classList.add('navHover');" onmouseout="this.classList.remove('navHover');" class="navLink" href="/settings">Settings</a>
                </div>
            </nav>
        </header>
        <main>
            <button type="button" id="toggleFilter" onclick="toggleFilter()" touchend="toggleFilter()">Show filter</button>
            <div id="filter">
                <label for="nrCheck">Use laps</label><input type="checkbox" id="nrCheck" checked="true" onchange="filterChanged();">
                <label for="atTimeCheck">Use time-stamp</label><input type="checkbox" id="atTimeCheck" value="false" onchange="filterChanged();">
            </div>
            <div id="times"></div>
        </main>


        <script>
            "use strict";
            let records = [];
            let csvVersion;
            let allTimes = [];
            const xhr = new XMLHttpRequest();
            let ready = true;

            class Record{
                constructor(val){
                    if(val != undefined && val != null){
                        this.atTime = val;
                    } else{
                        this.atTime = 0;
                    }
                }
                print(){
                    console.log("Rocord: atTime: " + this.atTime);
                }
            }

            class Filter{
                constructor(filter){
                    this.filter = filter;
                }

                filter(record){
                    for(let i = 0; i < filter.length; i++){
                        record = filter[i](record);
                        if(record == null){
                            return null;
                        }
                    }
                }
            }

            let filter = new Filter(function(record){
                record.atTime = msToString(record.atTime);
                record.duration = msToString(record.duration);
                return record;
            });

            setInterval(update, 200);

            function update(){
                if(ready){
                    ready = false;
                    xhr.open("GET", "/live", true);
                    xhr.send();
                    return true;
                }
                return false;
            }
            xhr.onreadystatechange = function(){
                if(xhr.readyState === 4){
                    if(xhr.status === 200){
                        processCsv(this.responseText);
                    } else if(xhr.status === 404){
                        console.log("Error 404 :( File not found");
                    }else{
                        console.log("Error in xhr");
                    }
                }
                ready = true;
            };

            function filterChanged(){
                csvVersion--;
            }
            
            function processCsv(csv){
                const elems = csv.split(",");
                const vals = [];
                for(let i = 0; i < elems.length; i++){
                    let number = parseInt(elems[i]);
                    if(!isNaN(number)){
                        vals[i] = number;
                    }
                }
                if(vals[0] !== csvVersion && vals.length > 1){
                    csvVersion = vals[0];
                    updateByCsv(vals);
                    console.log("xml changed!");
                }
            }

            function updateByCsv(vals){
                updateRecords(vals);
                document.getElementById("times").innerHTML = tableFromRecords(records);
            }

            function updateRecords(vals) {
                records = recordsFromVals(vals);
                sortRecords();
                generateRecordDurations();
            }

            function sortRecords(){
                records.sort(function(a, b){return a.atTime - b.atTime});
            }

            function generateRecordDurations(){
                let last = records[0].atTime;
                for (let i = 0; i < records.length; i++){
                    if(records[i] != undefined){
                        records[i].duration = records[i].atTime - last;
                        last = records[i].atTime;
                    }
                }
            }

            function recordsFromVals(vals){
                let records = [];
                for(let i = 1; i < vals.length; i++){
                    records[i] = new Record(vals[i]);
                }
                return records;
            }

            function tableFromRecords(records){
                let table1 = "<table><tr style='border-bottom: 2px solid black;'>" +
                (document.getElementById("nrCheck").checked ? "<td>Lap</td>" : "") +
                (document.getElementById("atTimeCheck").checked ? "<td><b>At Time</b></td>" : "") + "<td><b>Duration</b></td></tr>";
                let count = 1;
                for(let i = records.length - 1; i >= 0; i--){
                    if(records[i] == null){
                        continue;
                    }
                    table1 += "<tr" + ((records.length - i) % 2 === 1 ? " class='zebra'" : "") + ">" + (document.getElementById("nrCheck").checked ? ("<td>" + i + "</td>") : "") + (document.getElementById("atTimeCheck").checked ? ("<td>" + msToString(records[i].atTime) + "</td>") : "") + "<td>" + msToString(records[i].duration) + "</tr>";
                    count++;
                }
                return table1 + "</table>";
            }

            function msToString(ms){
                if(typeof ms != Number){
                    ms = parseInt(ms);
                }
                let out = "";
                let msString = "";
                if(ms >= 3600000){
                    out += parseInt(ms / 3600000) + ":"; //hours
                }
                if(ms >= 60000){
                    out = parseInt(ms / 60000) + ":";//minutes
                }
                if(ms % 1000 < 10){
                    msString = "00" + ms % 1000;
                } else if(ms % 1000 < 100){
                    msString = "0" + ms % 1000;
                } else{
                    msString = ms % 1000;
                }
                return out + parseInt((ms % 60000) / 1000) + "." + msString;//seconds.millis
            }
            
            function enterFullScreen(){
                document.getElementsByTagName("header")[0].classList.add("hideTop");
                document.getElementById("filter").style="height: 0em; opacity: 0;";
                filterOpened = !filterOpened;
            }

            function leaveFullScreen(){
                document.getElementsByTagName("header")[0].classList.remove("hideTop");
            }

            let filterOpened = false;
            function toggleFilter(){
                if(filterOpened){
                    document.getElementById("toggleFilter").innerText = "Show Filter";
                    document.getElementById("filter").style="height: 0em; opacity: 0;";
                }else{
                    document.getElementById("filter").style="height: 1.7em; opacity: 1;";
                    document.getElementById("toggleFilter").innerText = "Hide Filter";
                }
                filterOpened = !filterOpened;
            }
        </script>
    </body>
</html>
)=====";