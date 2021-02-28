const AWS = require('aws-sdk');
AWS.config.region = 'us-east-1';
const kinesis = new AWS.Kinesis();

const readline = require("readline");
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const streamName = 'RoverStream';

exports.main = async () =>{
    while(true){
        const input = await getInput('Enter data: ');
        await transmitData({
            state: input
        });
    }
}

exports.transmitData = async (data) => {
    console.log('Sending records...');
    const records = [
        {
            Data: JSON.stringify(data),
            PartitionKey: 'mypartition'
        }
    ];
    console.log(records);
    await putRecords(records);
}

async function getInput(question) {
    return new Promise((resolve, reject) => {
        rl.question(question, (data) => {
            resolve(data);
        });
    });
}

async function putRecords(records){
    return new Promise((resolve, reject) => {
        kinesis.putRecords(
            {
                Records: records,
                StreamName: streamName,
            },
            async (err, data) => {
                if (err) {
                    console.error(err);
                    reject(err);
                } else {
                    console.log('Success: ' + JSON.stringify(data));
                    resolve(data);
                }
            }
        );        
    });
}
