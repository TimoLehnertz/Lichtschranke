const char STYLE_CSS[] = R"=====(
*{
    margin: 0;
    padding: 0;
}
body{
    font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
}
h1{
    text-align: center;
}
a{
    text-decoration: none;
    color: black;
}
header{
    top: 0px;
    transition: top 1s;
    width: 100%;
    position: fixed;
    height: 50px;
    top: 0px;
}
main{
    margin-left: 5vw;
    margin-right: 5vw;
    min-width: 400px;
    margin-top: 70px;
}
#navCenter{
    width: 40%;
    margin-left: auto;
    margin-right: auto;
    min-width: 500px;
}
.navLink{
    font-size: 12pt;
    padding-top: 10px;
    display: inline-block;
    width: 22%;
    height: 40px;
    margin-left: 5%;
    margin-right: 5%;
    text-align: center;
    transition: height 100ms, color 100ms, background-color 100ms;
    background-color: #7d9bba;
}
.navHover{
    height: 50px;
    background-color: #2d3946;
    color: white;
}
.hideTop{
    top: -100px;
}
button{
    padding: 5px;
    border-radius: 2px;
}
#toggleFilter{
    float: left;
    height: 3em;
    margin-left: 10px;
    width: 7em;
}
#filter{
    margin-left: 10px;
    float: left;
    padding-top: 0.5em;
    height: 0em;
    border: solid 1px gray;
    border-radius: 1em;
    transition: height 200ms, opacity 200ms;
    overflow: hidden;
    opacity: 0;
}
#filter *{
    margin-left: 1em;
    margin-right: 1em;
    float: left;
}
#times{
    top: 75px;
    height: 70vh;
    width: 100%;
    overflow-y: auto;
    margin-left: auto;
    margin-right: auto;
}
table{
    margin-top: 15px;
    width: 100%;
    font-size: 13pt;
    border-collapse: collapse;   
}
td{
    padding: 5px;
    margin: 0px;
    text-align: center;
    border-right: 2px solid darkgray;
    border-left: 2px solid darkgray;
}
tr{
    border-bottom: 1px solid gray;
}
tr.zebra{
    background-color: #EEE;
}
@media (hover: none) and (pointer: coarse) {
    header{
        height: 80px;
    }
    
    main{
        margin-top: 100px;
    }
    
    #times{
        width: 100%;
        font-size: 40pt;
    }
    
    td{
        padding: 10px;
        font-size: 40pt;
    }
    
    input[type=checkbox] {
        /* All browsers except webkit*/
        transform: scale(3);
        /* Webkit browsers*/
        -webkit-transform: scale(3);
        margin-left: 50px;
        margin-right: 50px;
    }
    
    #filter{
        width: 70%;
    }
    
    #filter *{
        margin-left: 5px;
        margin-right: 5px;
        float: left;
        font-size: 25pt;
    }
    
    body{
        font-size: 30pt;
        margin-left: 0;
        margin-right: 0;
        width: 100%;
    }
    
    #toggleFilter{
        width: 20%;
        padding: 0;
        font-size: 20pt;
    }
    
    #navCenter{
        width: 100%;
    }
    
    .navLink{
        font-size: 30pt;
        height: 80px;
    }
}
)=====";