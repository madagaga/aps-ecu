
document.addEventListener("DOMContentLoaded", function () {
    loadPage('configuration'); // Load the default page
});

function loadPage(page) {
    const loaderDialog = document.getElementById('loader-dialog');
    const contentContainer = document.getElementById('content-container');
    loaderDialog.showModal();

    fetch(page + '.html')
        .then(response => response.text())
        .then(data => {
            contentContainer.innerHTML = data;
            if (page === 'configuration')
                loadConfiguration();
            else
                loadInverters();
        });
}

function loadInverters() {
    const loaderDialog = document.getElementById('loader-dialog');
    const inverterTemplate = `<h3>Inverter #id#</h3>
        <label for="inverter_#id#_serial">Serial:</label>
        <input type="text" id="inverter_#id#_serial" value="#inverter_serial#" required>        
        <label for="inverter_#id#_id">Id:</label>
        <input type="text" id="inverter_#id#_id" value="#inverter_id#" required>`;

    // a device that was never configured has no file yet: start from an
    // empty entry instead of rendering the 404 text
    fetch("/inverter_config.txt")
        .then(response => response.ok ? response.text() : '')
        .then(data => {
            const lines = data.split('\n');
            console.log(lines);

            lines.forEach((line, index) => {
                const tmp = document.createElement('div');
                tmp.innerHTML = inverterTemplate.replaceAll('#id#', index);
                const splitted = line.split(";");
                splitted.forEach(element => {
                    const [key, value] = element.split("=");
                    if (key !== '' && value !== undefined)
                        tmp.innerHTML = tmp.innerHTML.replaceAll('#inverter_' + key + '#', value);

                });
                // fields missing from the line stay empty, not "#inverter_...#"
                tmp.innerHTML = tmp.innerHTML.replace(/#inverter_[a-z]+#/g, '');
                document.forms[0].firstElementChild.appendChild(tmp);
            });
        })
        .finally(() => loaderDialog.close());
}

function loadConfiguration() {
    const loaderDialog = document.getElementById('loader-dialog');

    // no file on a device that was never configured: show an empty form
    fetch("/config.txt")
        .then(response => response.ok ? response.text() : '')
        .then(data => {
            const splitted = data.split(";");
            splitted.forEach(element => {
                const [key, value] = element.split("=");
                const input = key ? document.getElementById(key) : null;
                if (input && value !== undefined)
                    input.value = value;
            });
        })
        .finally(() => loaderDialog.close());
}

function sendConfiguration() {
    const data = [];
    data.push('wifi_ssid=' + document.getElementById('wifi_ssid').value);
    data.push('wifi_password=' + document.getElementById('wifi_password').value);
    data.push('mqtt_url=' + document.getElementById('mqtt_url').value);
    data.push('mqtt_port=' + document.getElementById('mqtt_port').value);
    data.push('mqtt_username=' + document.getElementById('mqtt_username').value);
    data.push('mqtt_password=' + document.getElementById('mqtt_password').value);
    data.push('mqtt_publish_topic=' + document.getElementById('mqtt_publish_topic').value);

    fetch("/config", {
        method: 'POST',
        body: data.join(';') + ';'
    })
        .then(response => response.text())
        .then(data => {
            console.log(data);
            alert("Configuration sauvegardée");
        });
    return false;
}

function sendInverterConfiguration() {
    const data = [];
    inputs = document.getElementsByTagName('input');
    for(let i=0;i<inputs.length;i+=2){
        data.push(`serial=${inputs[i].value};id=${inputs[i+1].value};`);
    }
    console.log(data);

    
    
    fetch("/inverters", {
        method: 'POST',
        body: data.join('\n')
    })
        .then(response => response.text())
        .then(data => {
            console.log(data);
            alert("Configuration sauvegardée");
        });
    return false;
}