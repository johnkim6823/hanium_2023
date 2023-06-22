import React, { useState } from 'react';
import axios from 'axios';
import ImgModal from './ImgModal';

const ShowDatabase = (props) => {
  const [dbData, setDbData] = useState([]); // array 형태로 전달되는 데이터를 받을 state
  const [showModal, setShowModal] = useState(false); // 모달 표시 여부를 관리하는 상태
  const [selectedCID, setSelectedCID] = useState(''); // 선택된 CID를 관리하는 상태

  const getData = () => {
    axios
      .get("http://localhost:3001/api/hello")//http://localhost:3001/api/2023_0417 router은 db연결
      .then((res) => {
        console.log(res);
        //setDbData([...res.data]); // response data를 받아서 dbData에 저장
      })
      .catch((err) => {
        console.log(err);결
      });
  };

  const handleModalOpen = (CID) => {
    setSelectedCID(CID); // 선택된 CID 설정
    setShowModal(true); // 모달 표시
  };

  const handleModalClose = () => {
    setShowModal(false); // 모달 닫기
  };

  const resultData = (data) => {
    return data.map((item) => (
      <tr key={item.CID}>
        <td>
          <span title="view image" onClick={() => handleModalOpen(item.CID)}>
            {item.CID}
          </span>
        </td>
        <td>{item.Hash}</td>
        <td>{item.Signed_Hash}</td>
        <td>{item.Verified}</td>
      </tr>
    ));
  };

  return (
    <div>
      <button onClick={getData}>데이터 보기</button>
      <table border="1">
        <thead>
          <tr>
            <th>CID</th>
            <th>Hash</th>
            <th>Signed_Hash</th>
            <th>Verified</th>
          </tr>
        </thead>
        <tbody>{resultData(dbData)}</tbody>
      </table>
      {showModal && (//showModal이 true일땐 modal을 보여줌 false일땐 안보여줌
        <ImgModal
          CID={selectedCID}
          show = {showModal}
          onClose={handleModalClose}
        />
      )}
    </div>
  );
};

export default ShowDatabase;
