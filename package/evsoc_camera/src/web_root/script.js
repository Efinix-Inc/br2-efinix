var currentPage = 1;
var totalPages = 1;

function fetching(url, data) {
  return fetch('/' + url, {  // Use the variable in the fetch request
      method: 'POST',
      headers: {
          'Content-Type': 'application/json'
      },
      body: JSON.stringify(data)
  })
  .then(r => {
    if (!r.ok) {
      throw new Error(`HTTP error! status: ${r.status}`);
    }
    return r.json();
  })
  .catch(err => console.log(err));
}

function updatePageNumber() {
  document.getElementById('page-num').innerText = `Page ${currentPage} of ${totalPages}`;
  sessionStorage.setItem("totalPages", totalPages);
  sessionStorage.setItem("currentPage", currentPage);
}

function checkButtonOpacity() {
  const prevButton = document.querySelector('.btn_prev');
  const nextButton = document.querySelector('.btn_next');

  console.log(`checkButtonOpacity: currentPage = ${currentPage}, totalPages = ${totalPages}`);
  Number(currentPage) === 1 ? prevButton.disabled = true : prevButton.disabled = false;
  Number(currentPage) === totalPages ? nextButton.disabled = true : nextButton.disabled = false;
  
  if ((Number(currentPage) === 1) && (Number(totalPages) === 1)) {
    prevButton.disabled = true;
    nextButton.disabled = true;
  }
}

function updatePagination() {
  updatePageNumber();
  checkButtonOpacity();
}

document.querySelector('.btn_prev').addEventListener('click', () => {
    currentPage--;
    fetching('btn_prev', {'btn_prev': currentPage})
    .then(r => {
      let img_list = JSON.parse(JSON.stringify(r));
      let page = JSON.parse(JSON.stringify(r.page));

      console.log(`GET /btn_prev ${img_list.capture}`);
      console.log(`page is ${page}`);
      totalPages = page;
      load_image(img_list.capture);
    })
    updatePagination();
});

document.querySelector('.btn_next').addEventListener('click', () => {
    currentPage++;

    fetching('btn_next', {'btn_next': currentPage})
    .then(r => {
      let img_list = JSON.parse(JSON.stringify(r));
      let page = JSON.parse(JSON.stringify(r.page));

      console.log(`GET /btn_next ${img_list.capture}`);
      console.log(`page is ${page}`);
      totalPages = page;
      load_image(img_list.capture);
    })
    updatePagination();
});

var E = function(id) { return document.getElementById(id); };

let btn_normal = E('btn_normal');
let btn_sw_grayscale = E('btn_sw_grayscale');
let btn_erosion = E('btn_erosion');

function clicked(clicked_btn) {
  let buttons = document.querySelectorAll('button');

  buttons.forEach(btn => {
    btn.classList.remove("button-click");
    btn.classList.add("button");
  })

  clicked_btn.classList.remove("button");
  clicked_btn.classList.add("button-click");

  sessionStorage.setItem('clicked_btn_id', clicked_btn.id);
}

const btns = [btn_normal, btn_sw_grayscale, btn_erosion];

btns.forEach(button => {
  button.onclick = () => {
    let id = button.id;
    clicked(button);
    fetching(id, {[id]: 1})
    .then(r => {
      console.log(`GET /${id}`);
    })
  }
});

document.getElementById('btn_capture').addEventListener('click', function() {
  fetch('/btn_capture', {
    method: 'POST',
    headers: {
        'Content-Type': 'application/json'
    },
    body: JSON.stringify({ 'btn_capture': 1 })
  })
  .then(r => r.json())
  .then(r => {
    console.log(`GET /btn_capture: ${JSON.stringify(r)}`);
    let cam = E('camera_output');
    cam.className = "blink";
    setTimeout(function() {
        cam.className = "camera_output";
    }, 1000);

    let p = document.getElementById('msg');
    let img_list = JSON.parse(JSON.stringify(r));
    let page = JSON.parse(JSON.stringify(r.page));

    p.innerHTML = 'Photo save as ' + img_list.capture[0];
    
    console.log(`page is ${page}`);
    load_image(img_list.capture);
    currentPage = 1;
    totalPages = page;
    updatePagination();
  
    console.log(`p = ${p.innerHTML}`);
    setTimeout(function() {
      p.innerHTML = '';
    }, 5000);

  })
  .catch(err => console.log(err));
});

function set_slider_digital_gain() {
  let s_analog_gain = document.getElementById("slider_digital_gain");
  let p_analog_gain = document.getElementById("p_digital_gain");
  p_analog_gain.innerHTML = s_analog_gain.value;

  s_analog_gain.oninput = function() {
    p_analog_gain.innerHTML = this.value;
    fetch('/digital_gain', {
      method: 'POST',
      headers: {
          'Content-Type': 'application/json'
      },
      body: JSON.stringify({ 'digital_gain': this.value })
    })
    .then(response => response.json())
    .then(data => {
      console.log(data);
      console.log(`value = ${this.value}`);
    })
    .catch((error) => {
      console.error('Error:', error);
    })
  }
}

function set_slider_exposure() {
  let s_exposure = document.getElementById("slider_exposure");
  let p_exposure = document.getElementById("p_exposure");
  p_exposure.innerHTML = s_exposure.value;

  s_exposure.oninput = function() {
    p_exposure.innerHTML = s_exposure.value;
    fetch('/exposure', {
      method: 'POST',
      headers: {
          'Content-Type': 'application/json'
      },
      body: JSON.stringify({ 'exposure': this.value })
    })
    .then(response => response.json())
    .then(data => {
      console.log(data);
      console.log(`value = ${this.value}`);
    })
    .catch((error) => {
      console.error('Error:', error);
    })
  }
}

function load_image(img_list, imgPerPage=4) {
  const gallery = E('div-gallery');
  const img_path = "/images/";

  // Remove existing gallery if it exists
  const existingGallery = E('gallery');
  if (existingGallery) {
    gallery.removeChild(existingGallery);
  }

  const div = createDiv("div-center", "gallery");

  img_list.slice(0, imgPerPage).forEach(img_name => {
    const imgDiv = createImageDiv(img_path, img_name);
    div.appendChild(imgDiv);
  });

  gallery.appendChild(div);
  sessionStorage.setItem("img_list", img_list);
}

function createDiv(className, id) {
  const div = document.createElement('div');
  div.className = className;
  div.id = id;
  return div;
}

function createImageDiv(img_path, img_name) {
  const gdiv = createDiv("gallery");
  const rdiv = createDiv("responsive", "rdiv");

  const img = document.createElement('img');
  img.src = img_path + img_name;

  const a = document.createElement('a');
  a.target = "_blank";
  a.href = img.src;
  a.appendChild(img);

  const desc = document.createElement('div');
  desc.innerHTML = img_name;
  desc.style.textAlign = "center";
  desc.style.padding = "15px";

  gdiv.appendChild(a);
  gdiv.appendChild(desc);
  rdiv.appendChild(gdiv);

  return rdiv;
}

window.onload = function() {
  fetching('load', {'load': currentPage})
  .then(r => {
      console.log(`GET /load: ${JSON.stringify(r)}`);
      totalPages = r.page;
      currentPage = sessionStorage.getItem("currentPage") || 1;
      currentPage = (currentPage <= 0 || currentPage > totalPages) ? 1 : currentPage;
      
      console.log(`on load: currentPage is ${currentPage}, totalPages is ${totalPages}`);
      updatePagination();

      let img_list = sessionStorage.getItem('img_list') ? sessionStorage.getItem('img_list').split(',') : r.capture;
      console.log(`img_list: ${img_list}`);

      load_image(img_list);
  })

  let btn_id = sessionStorage.getItem('clicked_btn_id') || 'btn_normal';
  clicked(E(btn_id));

  set_slider_digital_gain();
  set_slider_exposure();
}

