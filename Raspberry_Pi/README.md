# Install instructions
## ftmq_bridge
1. Copy the ftmq_bridge dir to '/etc/'
2. Copy ftmq_bridge.service to '/lib/systemd/system/'
3. Enable the ftmq_bridge service with 'sudo systemctl enable ftmq_bridge.service'
4. Start the ftmq_bridge service with 'sudo systemctl start ftmq_bridge.service'
5. Check if it's working with 'sudo systemctl status ftmq_bridge.service'
