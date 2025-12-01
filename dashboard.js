     function getStatusColor(temp) {
            if (temp < 18) return { color: '#90d0ef', status: 'Cool' };
            if (temp > 28) return { color: '#ff6b6b', status: 'Hot' };
            return { color: '#9bd58a', status: 'Normal' };
        }
        
        function getHumidityStatus(humidity) {
            if (humidity < 30) return { color: '#ffa726', status: 'Dry' };
            if (humidity > 70) return { color: '#90d0ef', status: 'Humid' };
            return { color: '#9bd58a', status: 'Comfortable' };
        }

        function updateDashboard(data) {
            document.getElementById('temperature').textContent = data.temperature.toFixed(1) + 'C';
            const tempStatus = getStatusColor(data.temperature);
            document.getElementById('temp-dot').style.backgroundColor = tempStatus.color;
            document.getElementById('temp-status').textContent = tempStatus.status;
            
            document.getElementById('humidity').textContent = data.humidity.toFixed(1) + '%';
            const humidityStatus = getHumidityStatus(data.humidity);
            document.getElementById('humidity-dot').style.backgroundColor = humidityStatus.color;
            document.getElementById('humidity-status').textContent = humidityStatus.status;
            
            document.getElementById('last-update').textContent = new Date().toLocaleTimeString();
        }

        function refreshData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    updateDashboard(data);
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('temperature').textContent = 'Error';
                    document.getElementById('humidity').textContent = 'Error';
                });
        }
        
        document.addEventListener('DOMContentLoaded', function() {
            refreshData(); 
     });
