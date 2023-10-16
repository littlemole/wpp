<!DOCTYPE html>
<html>
<head>
<title>Hello World</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link href="https://fonts.googleapis.com/css?family=Noto+Sans|Roboto+Mono|Titillium+Web" rel="stylesheet"> 
<link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.2.0/css/all.css" integrity="sha384-hWVjflwFxL6sNzntih27bfxkr27PmbbK/iSvJ+a4+0owXq79v+lsFkW54bOGbiDQ" crossorigin="anonymous">
<link rel="stylesheet" href="css/style.css">
<style>
</style>
</head>
<body>

<!-- Header -->
<div class="header">
  <b>repro-web</b>
  <span>modern reactive c++ for the web.</span>
  <a href="https://github.com/littlemole/wpp">wpp home</a>
  <a href="https://github.com/littlemole/wpp/tree/main/example">examples</a>
  <a href="https://github.com/littlemole/wpp/tree/main/example/hello_world">this example</a>
</div>

<div class="frame">
<div class="title">
    <h1>hello world</h1>
    <p>hello world example for modern reactive c++ for the web.</p>
</div>
</div>



<!-- The flexible grid (content) -->
<div class="row">
  <div class="side">
      <h2>Hello World Menu</h2>
  
<!-- Navigation Bar -->
     <div class="navbar">

      <ul class="nav">
       <li>
            <a href="/">home</a>
       </li>
       <li>
            <a href="/login">login</a>
       </li>
       <li>
            <a href="/register">register</a>
        </li>
        <li>
                <a href="/logout">logout</a>
            </li>
        </ul>
    
    </div>      
  </div>
  <div id="sitedivider"></div>
  <div class="main">
      <h2>Hello World Example</h2>

      <h5>This is a test</h5>
      <h1>Hello dear {{ username }}</h2>
      <p>you can logout <a href="/logout">here</a>. </p>

  </div>
</div>

<!-- Footer -->
<div class="footer">
		<!--
        <ul class="icons">
				<li><a href="#" class="icon fab fa-twitter"><span class="label">Twitter</span></a></li>
				<li><a href="#" class="icon fab fa-facebook"><span class="label">Facebook</span></a></li>
				<li><a href="#" class="icon fab fa-google-plus"><span class="label">Google+</span></a></li>
				<li><a href="#" class="icon fab fa-pinterest"><span class="label">Pinterest</span></a></li>
				<li><a href="#" class="icon fab fa-dribbble"><span class="label">Dribbble</span></a></li>
				<li><a href="#" class="icon fab fa-linkedin"><span class="label">LinkedIn</span></a></li>
			</ul>
		-->
			<div class="copyright">
				<ul class="menu">
					<li>&copy; littlemole. All rights reserved.</li>
				</ul>
			</div>
            <p>{{ version }}</p>            
</div>

</body>
</html>
